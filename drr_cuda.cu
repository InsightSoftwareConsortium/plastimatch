/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"

/****************************************************\
* Uncomment the line below to enable verbose output. *
* Enabling this should not nerf performance.         *
\****************************************************/
#define VERBOSE 1

/**********************************************************\
* Uncomment the line below to enable detailed performance  *
* reporting.  This measurement alters the system, however, *
* resulting in significantly slower kernel execution.      *
\**********************************************************/
#define TIME_KERNEL
#ifdef __DEVICE_EMULATION__
#define EMUSYNC __syncthreads()
#else
#define EMUSYNC
#endif

/*****************
*  C   #includes *
*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cuda.h>

#include "cuda_utils.h"
#include "drr_cuda.h"
#include "drr_cuda_p.h"
#include "drr_opts.h"
#include "file_util.h"
#include "math_util.h"
#include "proj_image.h"
#include "ray_trace_exact.h"
#include "volume.h"
#include "timer.h"

/* Textures */
texture<float, 1, cudaReadModeElementType> tex_img;
texture<float, 1, cudaReadModeElementType> tex_matrix;
texture<float, 1, cudaReadModeElementType> tex_coef;
texture<float, 3, cudaReadModeElementType> tex_3Dvol;

/* Inline functions */
inline __host__ __device__ 
float3 operator+ (float3 a, float3 b)
{
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline __host__ __device__ 
float3 operator- (float3 a, float3 b)
{
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline __host__ __device__ 
float3 operator* (float a, float3 b)
{
    return make_float3(a * b.x, a * b.y, a * b.z);
}
inline __host__ __device__ 
float3 operator* (float3 a, float3 b)
{
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline __host__ __device__ 
float3 operator/ (float a, float3 b)
{
    return make_float3(a / b.x, a / b.y, a / b.z);
}
inline __host__ __device__ 
int3 operator< (float3 a, float b)
{
    return make_int3 (a.x < b, a.y < b, a.z < b);
}
inline __host__ __device__ 
float3 fabsf3 (float3 a)
{
    return make_float3 (fabsf(a.x), fabsf(a.y), fabsf(a.z));
}

#define DRR_LEN_TOLERANCE 1e-6

/* From volume_limit.c */
__device__ int
volume_limit_clip_segment (
    float3 lower_limit,         /* INPUT:  The bounding box to clip to */
    float3 upper_limit,         /* INPUT:  The bounding box to clip to */
    float3 *ip1,                /* OUTPUT: Intersection point 1 */
    float3 *ip2,                /* OUTPUT: Intersection point 2 */
    float3 p1,                  /* INPUT:  Line segment point 1 */
    float3 p2                   /* INPUT:  Line segment point 2 */
)
{
    float3 ray, inv_ray;
    float3 alpha_low, alpha_high;
    int3 ploc;
    int3 is_parallel;

    ray = p2 - p1;
    inv_ray = 1.0f / ray;

    /* Find intersection configuration of ray base */
    /* -1 is POINTLOC_LEFT, 0 is POINTLOC_INSIDE, 1 is POINTLOC_RIGHT */
    ploc = make_int3 (-1, -1, -1);
    if (p1.x > upper_limit.x) {
	ploc.x = 1;
    } else if (p1.x > lower_limit.x) {
	ploc.x = 0;
    }
    if (p1.y > upper_limit.y) {
	ploc.y = 1;
    } else if (p1.y > lower_limit.y) {
	ploc.y = 0;
    }
    if (p1.z > upper_limit.z) {
	ploc.z = 1;
    } else if (p1.z > lower_limit.z) {
	ploc.z = 0;
    }

    /* Check if ray parallel to grid */
    is_parallel = fabsf3(ray) < DRR_LEN_TOLERANCE;

    /* Compute alphas for general configuration */
    alpha_low = (lower_limit - p1) * inv_ray;
    alpha_high = (upper_limit - p1) * inv_ray;

    /* Check case where ray is parallel to grid.  If any dimension is 
       parallel to grid, then p1 must be inside slap, otherwise there 
       is no intersection of segment and cube. */
    if (is_parallel.x) {
	if (!ploc.x) return 0;
	alpha_low.x = - FLT_MAX;
	alpha_high.x = + FLT_MAX;
    }
    if (is_parallel.y) {
	if (!ploc.y) return 0;
	alpha_low.y = - FLT_MAX;
	alpha_high.y = + FLT_MAX;
    }
    if (is_parallel.z) {
	if (!ploc.z) return 0;
	alpha_low.z = - FLT_MAX;
	alpha_high.z = + FLT_MAX;
    }

    /* Sort alpha values */
    

    return 1;
}

/* Main DRR function */
__global__ void
kernel_drr (
    float * dev_img, 
    int2 img_dim, 
    float2 ic, 
    float3 nrm, 
    float sad, 
    float scale, 
    float3 p1, 
    float3 ul_room, 
    float3 incr_r, 
    float3 incr_c, 
    float4 image_window, 
    float3 vol_offset, 
    int3 vol_dim, 
    float3 vol_pix_spacing
)
{
    extern __shared__ float sdata[];
    float3 vp;
    float3 p2;
    int i,j,k;
    int r, c;
    float vol;
    unsigned int tid = threadIdx.x;
    int idx;
    float outval;
    float3 r_tgt, tmp;
    int cols;

    /* Get coordinates of this image pixel */
    c = blockIdx.x * blockDim.x + threadIdx.x;
    r = blockIdx.y * blockDim.y + threadIdx.y;

    /* Compute ray */
    r_tgt = ul_room;
    tmp = r * incr_r;
    r_tgt = r_tgt + tmp;
    tmp = c * incr_c;
    p2 = r_tgt + tmp;

    /* Compute output location */
    cols = image_window.w - image_window.z + 1;
    idx = (c - image_window.z) + (r - image_window.x) * cols;

    /* Clip ray to volume */
    

    /* Loop through ray */

    /* Write output pixel value */
    dev_img[r*img_dim.x + c] = outval;
}

void*
drr_cuda_state_create (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
)
{
    Drr_cuda_state *state;
    Drr_kernel_args *kargs;

    state = (Drr_cuda_state *) malloc (sizeof(Drr_cuda_state));
    memset (state, 0, sizeof(Drr_cuda_state));

    state->kargs = kargs = (Drr_kernel_args*) malloc (sizeof(Drr_kernel_args));
    cudaMalloc ((void**) &state->dev_matrix, 12 * sizeof(float));
    cudaMalloc ((void**) &state->dev_kargs, sizeof(Drr_kernel_args));

    printf ("printf state = %p\n", state);
    printf ("printf state->kargs = %p\n", state->kargs);

    kargs->vol_offset.x = vol->offset[0];
    kargs->vol_offset.y = vol->offset[1];
    kargs->vol_offset.z = vol->offset[2];
    kargs->vol_dim.x = vol->dim[0];
    kargs->vol_dim.y = vol->dim[1];
    kargs->vol_dim.z = vol->dim[2];
    kargs->vol_pix_spacing.x = vol->pix_spacing[0];
    kargs->vol_pix_spacing.y = vol->pix_spacing[1];
    kargs->vol_pix_spacing.z = vol->pix_spacing[2];

    // prepare texture
    cudaChannelFormatDesc ca_descriptor;
    cudaExtent ca_extent;
    cudaArray *dev_3Dvol=0;

    ca_descriptor = cudaCreateChannelDesc<float>();
    ca_extent.width  = vol->dim[0];
    ca_extent.height = vol->dim[1];
    ca_extent.depth  = vol->dim[2];
    cudaMalloc3DArray (&dev_3Dvol, &ca_descriptor, ca_extent);
    cudaBindTextureToArray (tex_3Dvol, dev_3Dvol, ca_descriptor);

    cudaMemcpy3DParms cpy_params = {0};
    cpy_params.extent   = ca_extent;
    cpy_params.kind     = cudaMemcpyHostToDevice;
    cpy_params.dstArray = dev_3Dvol;

    //http://sites.google.com/site/cudaiap2009/cookbook-1#TOC-CUDA-3D-Texture-Example-Gerald-Dall
    // The pitched pointer is really tricky to get right. We give the
    // pitch of a row, then the number of elements in a row, then the
    // height, and we omit the 3rd dimension.
    cpy_params.srcPtr = make_cudaPitchedPtr ((void*)vol->img, 
	ca_extent.width * sizeof(float), ca_extent.width , ca_extent.height);

    cudaMemcpy3D (&cpy_params);

    cudaMalloc ((void**) &state->dev_img, 
	options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));

    cudaMalloc ((void**) &state->dev_coef, 
	7 * options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));
    cuda_utils_check_error ("Unable to allocate coef devmem");
    state->host_coef = (float*) malloc (
	7 * options->image_resolution[0] * options->image_resolution[1] 
	* sizeof(float));
		
    return (void*) state;
}

void
drr_cuda_state_destroy (
    void *void_state
)
{
    Drr_cuda_state *state = (Drr_cuda_state*) void_state;
    
    cudaFree (state->dev_img);
    cudaFree (state->dev_kargs);
    cudaFree (state->dev_matrix);
    cudaFree (state->dev_coef);
    free (state->host_coef);
    free (state->kargs);
}

void
drr_cuda_ray_trace_image (
    Proj_image *proj, 
    Volume *vol, 
    Volume_limit *vol_limit, 
    double p1[3], 
    double ul_room[3], 
    double incr_r[3], 
    double incr_c[3], 
    void *dev_state, 
    Drr_options *options
)
{
    Timer timer, total_timer;
    double time_kernel = 0;
    int i;

    // CUDA device pointers
    Drr_cuda_state *state = (Drr_cuda_state*) dev_state;
    Drr_kernel_args *kargs = state->kargs;

    // Start the timer
    plm_timer_start (&total_timer);
    plm_timer_start (&timer);

    // Load dynamic kernel arguments (different for each projection)
    kargs->img_dim.x = proj->dim[0];
    kargs->img_dim.y = proj->dim[1];
    kargs->ic.x = proj->pmat->ic[0];
    kargs->ic.y = proj->pmat->ic[1];
    kargs->nrm.x = proj->pmat->nrm[0];
    kargs->nrm.y = proj->pmat->nrm[1];
    kargs->nrm.z = proj->pmat->nrm[2];
    kargs->sad = proj->pmat->sad;
    kargs->sid = proj->pmat->sid;
    for (i = 0; i < 12; i++) {
	kargs->matrix[i] = (float) proj->pmat->matrix[i];
    }
    kargs->p1.x = p1[0];
    kargs->p1.y = p1[1];
    kargs->p1.z = p1[2];
    kargs->ul_room.x = ul_room[0];
    kargs->ul_room.y = ul_room[1];
    kargs->ul_room.z = ul_room[2];
    kargs->incr_r.x = incr_r[0];
    kargs->incr_r.y = incr_r[1];
    kargs->incr_r.z = incr_r[2];
    kargs->incr_c.x = incr_c[0];
    kargs->incr_c.y = incr_c[1];
    kargs->incr_c.z = incr_c[2];
    kargs->image_window.x = options->image_window[0];
    kargs->image_window.y = options->image_window[1];
    kargs->image_window.z = options->image_window[2];
    kargs->image_window.w = options->image_window[3];

    printf ("ul_room = %f %f %f\n", ul_room[0], ul_room[1], ul_room[2]);

    cudaMemcpy (state->dev_matrix, kargs->matrix, sizeof(kargs->matrix), 
	cudaMemcpyHostToDevice);
    cudaBindTexture (0, tex_matrix, state->dev_matrix, sizeof(kargs->matrix));

    // Thread Block Dimensions
    int tBlock_x = 16;
    int tBlock_y = 16;

    // Each element in the image gets 1 thread
    int blocksInX = (vol->dim[0]+tBlock_x-1)/tBlock_x;
    int blocksInY = (vol->dim[1]+tBlock_y-1)/tBlock_y;
    dim3 dimGrid  = dim3(blocksInX, blocksInY);
    dim3 dimBlock = dim3(tBlock_x, tBlock_y);

    // Note: proj->img AND proj->matrix are passed via texture memory

    //int smemSize = vol->dim[0]  * sizeof(float);

    printf ("Preprocessing time: %f secs\n", plm_timer_report (&timer));
    plm_timer_start (&timer);

    // Invoke ze kernel  \(^_^)/
    kernel_drr<<< dimGrid, dimBlock>>> (
	state->dev_img, 
	kargs->img_dim,
	kargs->ic,
	kargs->nrm,
	kargs->sad,
	kargs->scale,
	kargs->p1, 
	kargs->ul_room, 
	kargs->incr_r, 
	kargs->incr_c, 
	kargs->image_window, 
	kargs->vol_offset,
	kargs->vol_dim,
	kargs->vol_pix_spacing);

    printf ("Kernel time: %f secs\n", plm_timer_report (&timer));
    plm_timer_start (&timer);

    cuda_utils_check_error("Kernel Panic!");

#if defined (TIME_KERNEL)
    // CUDA kernel calls are asynchronous...
    // In order to accurately time the kernel
    // execution time we need to set a thread
    // barrier here after its execution.
    cudaThreadSynchronize();
#endif

    time_kernel += plm_timer_report (&timer);

    // Unbind the image and projection matrix textures
    //cudaUnbindTexture( tex_img );
    cudaUnbindTexture (tex_matrix);
    cudaUnbindTexture (tex_coef);

    // Copy reconstructed volume from device to host
    //cudaMemcpy( vol->img, dev_vol, vol->npix * vol->pix_size, cudaMemcpyDeviceToHost );
    cudaMemcpy (proj->img, state->dev_img, 
	proj->dim[0] * proj->dim[1] * sizeof(float), 
	cudaMemcpyDeviceToHost);
    cuda_utils_check_error("Error: Unable to retrieve data volume.");
}
