/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _bspline_cuda_kernels_h_
#define _bspline_cuda_kernels_h_

/* Function prototypes of kernels */
__global__ void kernel_deinterleave(
				int num_values,
				float* input,
				float* out_x,
				float* out_y,
				float* out_z);

__global__ void kernel_pad_64(
			float* input,
			float* output,
			int3 vol_dim,
			int3 tile_dim);

__global__ void kernel_pad(
			float* input,
			float* output,
			int3 vol_dim,
			int3 tile_dim);


__global__ void kernel_row_to_tile_major(
			float* input,
			float* output,
			int3 vol_dim,
			int3 tile_dim);


__global__ void kernel_bspline_mse_2_condense_64_texfetch(
				float* cond_x,		// Return: condensed dc_dv_x values
				float* cond_y,		// Return: condensed dc_dv_y values
				float* cond_z,		// Return: condensed dc_dv_z values
				float* dc_dv_x,		// Input : dc_dv_x values
				float* dc_dv_y,		// Input : dc_dv_y values
				float* dc_dv_z,		// Input : dc_dv_z values
				int* LUT_Tile_Offsets,	// Input : tile offsets
				int* LUT_Knot,		// Input : linear knot indicies
				int pad,		// Input : amount of tile padding
				int4 tile_dim,		// Input : dims of tiles
				float one_over_six);	// Input : Precomputed since GPU division is slow


__global__ void kernel_bspline_mse_2_condense_64(
				float* cond_x,		// Return: condensed dc_dv_x values
				float* cond_y,		// Return: condensed dc_dv_y values
				float* cond_z,		// Return: condensed dc_dv_z values
				float* dc_dv_x,		// Input : dc_dv_x values
				float* dc_dv_y,		// Input : dc_dv_y values
				float* dc_dv_z,		// Input : dc_dv_z values
				int* LUT_Tile_Offsets,	// Input : tile offsets
				int* LUT_Knot,		// Input : linear knot indicies
				int pad,		// Input : amount of tile padding
				int4 tile_dim,		// Input : dims of tiles
				float one_over_six);	// Input : Precomputed since GPU division is slow


__global__ void kernel_bspline_mse_2_condense(
				float* cond_x,		// Return: condensed dc_dv_x values
				float* cond_y,		// Return: condensed dc_dv_y values
				float* cond_z,		// Return: condensed dc_dv_z values
				float* dc_dv_x,		// Input : dc_dv_x values
				float* dc_dv_y,		// Input : dc_dv_y values
				float* dc_dv_z,		// Input : dc_dv_z values
				int* LUT_Tile_Offsets,	// Input : tile offsets
				int* LUT_Knot,		// Input : linear knot indicies
				int pad,		// Input : amount of tile padding
				int4 tile_dim,		// Input : dims of tiles
				float one_over_six);	// Input : Precomputed since GPU division is slow


__global__ void kernel_bspline_mse_2_reduce(
				float* grad,		// Return: interleaved dc_dp values
				float* cond_x,		// Input : condensed dc_dv_x values
				float* cond_y,		// Input : condensed dc_dv_y values
				float* cond_z);		// Input : condensed dc_dv_z values


__device__ float obtain_spline_basis_function(float one_over_six,
						  int t_idx, 
						  int vox_idx, 
						  int vox_per_rgn);



__global__ void
bspline_cuda_score_j_mse_kernel1 
(
 float  *dc_dv_x,	// OUTPUT
 float  *dc_dv_y,	// OUTPUT
 float  *dc_dv_z,	// OUTPUT
 float  *score,		// OUTPUT
 float  *coeff,		// INPUT
 float  *fixed_image,	// INPUT
 float  *moving_image,	// INPUT
 float  *moving_grad,	// INPUT
 int3   volume_dim,	// x, y, z dimensions of the volume in voxels
 float3 img_origin,	// Image origin (in mm)
 float3 img_spacing,	// Image spacing (in mm)
 float3 img_offset,	// Offset corresponding to the region of interest
 int3   roi_offset,	// Position of first vox in ROI (in vox)
 int3   roi_dim,	// Dimension of ROI (in vox)
 int3   vox_per_rgn,	// Knot spacing (in vox)
 float3 pix_spacing,	// Dimensions of a single voxel (in mm)
 int3   rdims,		// # of regions in (x,y,z)
 int3   cdims,
 int	pad,
 float*   skipped);



__global__ void bspline_cuda_score_g_mse_kernel1 
(
 float  *dc_dv,
 float  *score,		
 float  *coeff,
 float  *fixed_image,
 float  *moving_image,
 float  *moving_grad,
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 int3   rdims,			// # of regions in (x,y,z)
 int3   cdims);

__global__ void bspline_cuda_score_g_mse_kernel2 
(
 float *dc_dv,
 float *grad,
 int   num_threads,
 int3  rdims,
 int3  cdims,
 int3  vox_per_rgn);

__global__ void bspline_cuda_score_g_mse_kernel1_low_mem 
(
 float  *dc_dv,
 float  *score,			
 int    tile_index,		// Linear index of the starting tile
 int    num_tiles,       // Number of tiles to work on per kernel launch
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 int3   rdims,			// # of regions in (x,y,z)
 int3   cdims);

__global__ void bspline_cuda_score_f_mse_compute_score 
(
 float  *dc_dv,
 float  *score,
 float  *diffs,
 float  *mvrs,
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 int3   rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_f_compute_dc_dv
(
 float *dc_dv,	
 int3  volume_dim,		// x, y, z dimensions of the volume in voxels
 int3  vox_per_rgn,	    // Knot spacing (in vox)
 int3  roi_offset,	    // Position of first vox in ROI (in vox)
 int3  roi_dim,			// Dimension of ROI (in vox)
 int3  rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_f_mse_kernel1_v2 
(
 float  *dc_dv_x,
 float  *dc_dv_y,
 float  *dc_dv_z,
 float  *score,			
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 int3   rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_f_mse_kernel1 
(
 float  *dc_dv,
 float  *score,	
 int    *gpu_c_lut,
 float  *gpu_q_lut,
 float  *coeff,
 float  *fixed_image,
 float  *moving_image,
 float  *moving_grad,
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 int3   rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_f_mse_kernel1_low_mem 
(
 float  *dc_dv,
 float  *score,			
 int3   p,				// Offset of the tile in the volume (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 float3 rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_f_mse_kernel2_v2 
(
 float *grad,
 int   num_threads,
 int3  rdims,
 int3  cdims,
 int3  vox_per_rgn);

__global__ void bspline_cuda_score_f_mse_kernel2 
(
 float *dc_dv,
 float *grad,
 int   num_threads,
 int3  rdims,
 int3  cdims,
 int3  vox_per_rgn);

__global__ void bspline_cuda_score_f_mse_kernel2_nk 
(
 float *dc_dv,
 float *grad,
 int   num_threads,
 int3  rdims,
 int3  cdims,
 int3  vox_per_rgn);

__global__ void bspline_cuda_score_e_mse_kernel1a 
(
 float  *dc_dv,
 float  *score,
 float3 rdims,			// Number of tiles/regions in x, y, and z
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing);		// Dimensions of a single voxel (in mm)

__global__ void bspline_cuda_score_e_mse_kernel1b 
(
 float  *dc_dv,
 float  *score,
 int3   sidx,			// Current "set index" given in x, y and z
 float3 rdims,			// Number of tiles/regions in x, y, and z
 int3   sdims,           // Dimensions of the set in tiles (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 int    total_vox_per_rgn,
 float3 pix_spacing);		// Dimensions of a single voxel (in mm)

__global__ void bspline_cuda_score_e_mse_kernel1 
(
 float  *dc_dv,
 float  *score,
 int3   sidx,			// Current "set index" given in x, y and z
 float3 rdims,			// Number of tiles/regions in x, y, and z
 int3   sdims,           // Dimensions of the set in tiles (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 int    total_vox_per_rgn,
 float3 pix_spacing);		// Dimensions of a single voxel (in mm)

__global__ void bspline_cuda_score_e_mse_kernel2_by_sets
(
 float  *dc_dv,
 float  *grad,
 float  *gpu_q_lut,
 int3   sidx,
 int3   sdims,
 float3 rdims,
 int3   vox_per_rgn,
 int    threads_per_tile,
 int    num_threads);

__global__ void bspline_cuda_score_e_mse_kernel2_by_tiles 
(
 float  *dc_dv,
 float  *grad,
 float  *gpu_q_lut,
 int    num_threads,
 int3   p,
 float3 rdims,
 int    offset,
 int3   vox_per_rgn,
 int    total_vox_per_rgn); // Volume of a tile in voxels)

__global__ void bspline_cuda_score_e_mse_kernel2_by_tiles_v2 
(
 float  *dc_dv,
 float  *grad,
 float  *gpu_q_lut,
 int    num_threads,
 int3   p,
 float3 rdims,
 int    offset,
 int3   vox_per_rgn,
 int    threadsPerControlPoint);

__global__ void bspline_cuda_score_d_mse_kernel1 
(
 float  *dc_dv,
 float  *score,			
 int3   p,				// Offset of the tile in the volume (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 float3 rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_d_mse_kernel1_v2 
(
 float  *dc_dv,
 float  *score,			
 int3   p,				// Offset of the tile in the volume (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 float3 rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_d_mse_kernel1_v3 
(
 float  *dc_dv,
 float  *score,			
 int3   p,				// Offset of the tile in the volume (x, y and z)
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// Image origin (in mm)
 float3 img_spacing,     // Image spacing (in mm)
 float3 img_offset,		// Offset corresponding to the region of interest
 int3   roi_offset,	    // Position of first vox in ROI (in vox)
 int3   roi_dim,			// Dimension of ROI (in vox)
 int3   vox_per_rgn,	    // Knot spacing (in vox)
 float3 pix_spacing,		// Dimensions of a single voxel (in mm)
 float3 rdims);			// # of regions in (x,y,z)

__global__ void bspline_cuda_score_d_mse_kernel2 
(
 float  *dc_dv,
 float  *grad,
 float  *gpu_q_lut,
 int    num_threads,
 int3   p,
 float3 rdims,
 int3   vox_per_rgn);

__global__ void bspline_cuda_score_d_mse_kernel2_v2 
(
 float* grad,
 int    num_threads,
 int3   p,
 float3 rdims,
 int3   vox_per_rgn,
 int    threadsPerControlPoint);

__global__ void bspline_cuda_compute_dxyz_kernel
(
 int   *c_lut,
 float *q_lut,
 float *coeff,
 int3 volume_dim,
 int3 vox_per_rgn,
 float3 rdims,
 float *dx,
 float *dy,
 float *dz
 );

__global__ void bspline_cuda_compute_diff_kernel 
(
 float* fixed_image,
 float* moving_image,
 float* dx,
 float* dy,
 float* dz,
 float* diff,
 int*   valid_voxels,
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 float3 img_origin,		// x, y, z coordinates for the image origin
 float3 pix_spacing,		// Dimensions of a single voxel in millimeters
 float3 img_offset);		// Offset corresponding to the region of interest

__global__ void bspline_cuda_compute_dc_dv_kernel 
(
 float  *fixed_image,
 float  *moving_image,
 float  *moving_grad,
 int    *c_lut,
 float  *q_lut,
 float  *dx,
 float  *dy,
 float  *dz,
 float  *diff,
 float  *dc_dv_x,
 float  *dc_dv_y,
 float  *dc_dv_z,
 // float  *grad,
 int    *valid_voxels,
 int3   volume_dim,		// x, y, z dimensions of the volume in voxels
 int3   vox_per_rgn,
 float3 rdims,
 float3 img_origin,		// x, y, z coordinates for the image origin
 float3 pix_spacing,		// Dimensions of a single voxel in millimeters
 float3 img_offset);		// Offset corresponding to the region of interest

__global__ void sum_reduction_kernel
(
 float *idata, 
 float *odata, 
 int   num_elems);

__global__ void bspline_cuda_compute_score_kernel
(
 float *idata, 
 float *odata, 
 int   *valid_voxels, 
 int   num_elems);

__global__ void sum_reduction_last_step_kernel
(
 float *idata,
 float *odata,
 int   num_elems);

__global__ void bspline_cuda_update_grad_kernel
(
 float *grad,
 int num_vox,
 int num_elems);

__global__ void bspline_cuda_compute_grad_mean_kernel
(
 float *idata,
 float *odata,
 int num_elems);

__global__ void bspline_cuda_compute_grad_norm_kernel
(
 float *idata,
 float *odata,
 int num_elems);

__global__ void k_bspline_cuda_MI_a_hist_fix
(
 float *f_hist_seg,
 float *fixed,
 float offset,
 float delta,
 long bins,
 int nthreads);


__global__ void k_bspline_cuda_MI_a_hist_mov (
 float* m_hist_seg,	// partial histogram (moving image)
 float* fixed,		// fixed  image voxels
 float* moving,		// moving image voxels
 float offset,		// histogram offset
 float delta,		// histogram delta
 long bins,		// # histogram bins
 int3 vpr,		// voxels per region
 int3 fdim,		// fixed  image dimensions
 int3 mdim,		// moving image dimensions
 int3 rdim,		//       region dimensions
 float3 img_origin,	// image origin
 float3 img_spacing,	// image spacing
 float3 mov_offset,	// moving image offset
 float3 mov_ps,		// moving image pixel spacing
 int* c_lut,		// DEBUG
 float* q_lut,		// DEBUG
 float* coeff,		// DEBUG
 int nthreads);		// # threads (to be removed)


__global__ void k_bspline_cuda_MI_a_hist_jnt (
 float* j_hist_seg,	// partial histogram (joint)
 float* fixed,		// fixed  image voxels
 float* moving,		// moving image voxels
 float f_offset,	// fixed histogram offset
 float m_offset,	// moving histogram offset
 float f_delta,		// fixed histogram delta
 float m_delta,		// moving histogram delta
 long m_bins,		// # moving histogram bins
 long j_bins,		// # joint  histogram bins
 int3 vpr,		// voxels per region
 int3 fdim,		// fixed  image dimensions
 int3 mdim,		// moving image dimensions
 int3 rdim,		//       region dimensions
 float3 img_origin,	// image origin
 float3 img_spacing,	// image spacing
 float3 mov_offset,	// moving image offset
 float3 mov_ps,		// moving image pixel spacing
 int* c_lut,		// DEBUG
 float* q_lut,		// DEBUG
 float* coeff,		// DEBUG
 int nthreads);		// # threads (to be removed)


__global__ void k_bspline_cuda_MI_a_hist_fix_merge (
 float *f_hist,
 float *f_hist_seg,
 long num_seg_hist);

#endif
