/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "drr_opencl.h"
#include "drr_opencl_p.h"
#include "drr.h"
#include "drr_opts.h"
#include "file_util.h"
#include "math_util.h"
#include "opencl_util.h"
#include "opencl_util_nvidia.h"
#include "plm_timer.h"
#include "proj_image.h"
#include "proj_matrix.h"
#include "volume.h"
#include "volume_limit.h"

void*
drr_opencl_state_create (
    Proj_image *proj,
    Volume *vol,
    Drr_options *options
)
{
    printf ("drr_opencl_state_create: 1\n");
    LOAD_LIBRARY (libplmopencl);
    printf ("drr_opencl_state_create: 2\n");
    LOAD_SYMBOL (opencl_open_device, libplmopencl);
    LOAD_SYMBOL (opencl_load_programs, libplmopencl);
    LOAD_SYMBOL (opencl_kernel_create, libplmopencl);
    LOAD_SYMBOL (opencl_buf_create, libplmopencl);
    printf ("drr_opencl_state_create: 3\n");

    Drr_opencl_state *dev_state;
    dev_state = (Drr_opencl_state*) malloc (sizeof (Drr_opencl_state));
    printf ("drr_opencl_state_create: 4\n");

    /* Set up devices and kernels */
    opencl_open_device (&dev_state->ocl_dev);
    opencl_load_programs (&dev_state->ocl_dev, "drr_opencl.cl");
    opencl_kernel_create (&dev_state->ocl_dev, "kernel_drr");
    printf ("drr_opencl_state_create: 5\n");

    /* Set up device memory */
    dev_state->ocl_buf_img = opencl_buf_create (
        &dev_state->ocl_dev, 
        CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, 
        proj->dim[1] * proj->dim[0] * sizeof(float),
        0
    );

    dev_state->ocl_buf_vol = opencl_buf_create (
        &dev_state->ocl_dev, 
        CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, 
        vol->pix_size * vol->npix,
        vol->img
    );

    printf ("drr_opencl_state_create: 6\n");
    UNLOAD_LIBRARY (libplmopencl);
    printf ("drr_opencl_state_create: 7\n");
    return (void*) dev_state;
}

void
drr_opencl_state_destroy (void *dev_state)
{
    if (!dev_state) return;

    free (dev_state);
}

void
drr_opencl_ray_trace_image (
    Proj_image *proj, 
    Volume *vol, 
    Volume_limit *vol_limit, 
    double p1[3], 
    double ul_room[3], 
    double incr_r[3], 
    double incr_c[3], 
    void *dev_state_v, 
    Drr_options *options
)
{
    LOAD_LIBRARY (libplmopencl);
    LOAD_SYMBOL (opencl_set_kernel_args, libplmopencl);
    LOAD_SYMBOL (opencl_kernel_enqueue, libplmopencl);
    LOAD_SYMBOL (opencl_buf_read, libplmopencl);

    Drr_opencl_state *dev_state = (Drr_opencl_state*) dev_state_v;
    Proj_matrix *pmat = proj->pmat;
    cl_float2 ocl_ic;
    cl_float4 ocl_p1, ocl_ul_room, ocl_incr_r, ocl_incr_c;
    cl_float4 ocl_nrm, ocl_lower_limit, ocl_upper_limit;
    cl_float ocl_sad;

    /* Copy ic to device (convert from double to float) */
    opencl_idx(ocl_ic,0) = proj->pmat->ic[0];
    opencl_idx(ocl_ic,1) = proj->pmat->ic[1];

    /* Copy p1 to device (convert from double to float) */
    opencl_idx(ocl_p1,0) = p1[0];
    opencl_idx(ocl_p1,1) = p1[1];
    opencl_idx(ocl_p1,2) = p1[2];

    /* Copy ul_room to device (convert from double to float) */
    opencl_idx(ocl_ul_room,0) = ul_room[0];
    opencl_idx(ocl_ul_room,1) = ul_room[1];
    opencl_idx(ocl_ul_room,2) = ul_room[2];

    /* Copy incr_r to device (convert from double to float) */
    opencl_idx(ocl_incr_r,0) = incr_r[0];
    opencl_idx(ocl_incr_r,1) = incr_r[1];
    opencl_idx(ocl_incr_r,2) = incr_r[2];

    /* Copy incr_c to device (convert from double to float) */
    opencl_idx(ocl_incr_c,0) = incr_c[0];
    opencl_idx(ocl_incr_c,1) = incr_c[1];
    opencl_idx(ocl_incr_c,2) = incr_c[2];

    /* Copy nrm to device (convert from double to float) */
    opencl_idx(ocl_nrm,0) = pmat->nrm[0];
    opencl_idx(ocl_nrm,1) = pmat->nrm[1];
    opencl_idx(ocl_nrm,2) = pmat->nrm[2];

    /* Copy lower_limit to device (convert from double to float) */
    opencl_idx(ocl_lower_limit,0) = vol_limit->lower_limit[0];
    opencl_idx(ocl_lower_limit,1) = vol_limit->lower_limit[1];
    opencl_idx(ocl_lower_limit,2) = vol_limit->lower_limit[2];
    
    /* Copy upper_limit to device (convert from double to float) */
    opencl_idx(ocl_upper_limit,0) = vol_limit->upper_limit[0];
    opencl_idx(ocl_upper_limit,1) = vol_limit->upper_limit[1];
    opencl_idx(ocl_upper_limit,2) = vol_limit->upper_limit[2];

    /* Convert sad from double to float */
    ocl_sad = proj->pmat->sad;

    /* Set drr kernel arguments */
    opencl_set_kernel_args (
	&dev_state->ocl_dev, 
	sizeof (cl_mem), &dev_state->ocl_buf_img[0], 
	sizeof (cl_mem), &dev_state->ocl_buf_vol[0], 
	sizeof (cl_int4), vol->dim, 
	sizeof (cl_float4), vol->offset, 
	sizeof (cl_float4), vol->pix_spacing, 
	sizeof (cl_int2), proj->dim, 
	sizeof (cl_float2), &ocl_ic, 
	sizeof (cl_int4), options->image_window, 
	sizeof (cl_float4), &ocl_p1,
	sizeof (cl_float4), &ocl_ul_room,
	sizeof (cl_float4), &ocl_incr_r,
	sizeof (cl_float4), &ocl_incr_c,
	sizeof (cl_float4), &ocl_nrm,
	sizeof (cl_float4), &ocl_lower_limit,
	sizeof (cl_float4), &ocl_upper_limit,
	sizeof (cl_float), &ocl_sad, 
	sizeof (cl_float), &options->scale,
	(size_t) 0
    );

    /* Compute workgroup size */
    /* (Max local_work_size for my ATI RV710 is 128) */
    size_t local_work_size = 128;
    size_t global_work_size = (float) proj->dim[0] * proj->dim[1];

    /* Invoke kernel */
    opencl_kernel_enqueue (&dev_state->ocl_dev, 
	global_work_size, local_work_size);

    /* Read back results */
    opencl_buf_read (&dev_state->ocl_dev, dev_state->ocl_buf_img, 
	sizeof (float) * proj->dim[0] * proj->dim[1], proj->img);
}
