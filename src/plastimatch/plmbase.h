/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _plmbase_h_
#define _plmbase_h_

#include "plmbase_config.h"
#include "sys/plm_int.h"

// opaque types
class Bspline_xform;
class Metadata;
class Plm_image;
class Proj_image;
class Proj_matrix;
class Slice_index;
class Slice_index;
class Rtss;
class Volume;
class Volume_header;
typedef struct raw_pointset Raw_pointset;
typedef struct rpl_volume Rpl_volume;
typedef struct xpm_brush_s xpm_brush;
typedef struct xpm_struct_s xpm_struct;
typedef struct volume_limit Volume_limit;

// Callback definitions 
typedef void (*Ray_trace_callback) (
    void *callback_data, 
    int vox_index, 
    double vox_len, 
    float vox_value);

// enumerated types
enum xpm_brushes {
	XPM_BOX,
	XPM_CIRCLE
};


// API
#if GDCM_VERSION_1
/* gdcm1_dose.cxx */
API bool gdcm1_dose_probe (const char *dose_fn);
API Plm_image* gdcm1_dose_load (
        Plm_image *pli,
        const char *dose_fn,
        const char *dicom_dir
);
API void gdcm1_dose_save (
        Plm_image *pli, 
        const Metadata *meta, 
        const Slice_index *rdd, 
        const char *dose_fn);

/* gdcm1_series.cxx */
API void gdcm1_series_test (char *dicom_dir);

/* gdcm1_rtss.cxx */
API bool gdcm_rtss_probe (const char *rtss_fn);
API void gdcm_rtss_load (
        Rtss *rtss,             /* Output: this gets loaded into */
        Slice_index *rdd,       /* Output: this gets updated too */
        Metadata *meta,         /* Output: this gets updated too */
        const char *rtss_fn    /* Input: the file that gets read */
);
API void gdcm_rtss_save (
        Rtss *rtss,             /* Input: this is what gets saved */
        Slice_index *rdd,       /* Input: need to look at this too */
        char *rtss_fn           /* Input: name of file to write to */
);
#endif

/* bspline_xform.cxx */
API void bspline_interpolate_vf (Volume* interp, const Bspline_xform* bxf);
API void bspline_transform_point (
        float point_out[3], /* Output coordinate of point */
        Bspline_xform* bxf, /* Bspline transform coefficients */
        float point_in[3],  /* Input coordinate of point */
        int linear_interp   /* 1 = trilinear, 0 = nearest neighbors */
);
API void bspline_xform_set_default (Bspline_xform* bxf);
API void bspline_xform_initialize (
        Bspline_xform* bxf,	          /* Output: bxf is initialized */
        float img_origin[3],          /* Image origin (in mm) */
        float img_spacing[3],         /* Image spacing (in mm) */
        plm_long img_dim[3],          /* Image size (in vox) */
        plm_long roi_offset[3],       /* Position of first vox in ROI (in vox) */
        plm_long roi_dim[3],	      /* Dimension of ROI (in vox) */
        plm_long vox_per_rgn[3],      /* Knot spacing (in vox) */
        float direction_cosines[9]    /* Direction cosines */
);
API void bspline_xform_free (Bspline_xform* bxf);
API Bspline_xform* bspline_xform_load (const char* filename);
API void bspline_xform_save (Bspline_xform* bxf, const char* filename);
API void bspline_set_coefficients (Bspline_xform* bxf, float val);

/* Debugging routines */
gpuit_EXPORT
void
bspline_xform_dump_coeff (Bspline_xform* bxf, const char* fn);

gpuit_EXPORT
void
bspline_xform_dump_luts (Bspline_xform* bxf);


/* hnd_io.cxx */
API void hnd_load (
        Proj_image *proj,
        const char *fn,
        const double xy_offset[2]
);

/* raw_pointset.cxx */
API void pointset_add_point (
        Raw_pointset *ps,
        float lm[3]
);
API void pointset_add_point_noadjust (
        Raw_pointset *ps,
        float lm[3]
);
API Raw_pointset *pointset_create (void);
API void pointset_debug (Raw_pointset* ps);
API void pointset_destroy (Raw_pointset *ps);
API Raw_pointset* pointset_load (const char *fn);
API void pointset_resize (
        Raw_pointset *ps,
        int new_size
);
API void pointset_save (
        Raw_pointset* ps,
        const char *fn
);
API void pointset_save_fcsv_by_cluster (
        Raw_pointset* ps,
        int *clust_id,
        int which_cluster,
        const char *fn
);

/* ray_trace_exact.cxx */
API void ray_trace_exact (
        Volume *vol,                  /* Input: volume */
        Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
        Ray_trace_callback callback,  /* Input: callback function */
        void *callback_data,          /* Input: callback function private data */
        double *p1in,                 /* Input: start point for ray */
        double *p2in                  /* Input: end point for ray */
);

/* ray_trace_uniform.cxx */
API void ray_trace_uniform (
        Volume *vol,                  /* Input: volume */
        Volume_limit *vol_limit,      /* Input: min/max coordinates of volume */
        Ray_trace_callback callback,  /* Input: callback function */
        void *callback_data,          /* Input: callback function private data */
        double *p1in,                 /* Input: start point for ray */
        double *p2in,                 /* Input: end point for ray */
        float ray_step                /* Input: uniform step size */
);

/* mha_io.cxx */
API Volume* read_mha (const char* filename);
API void write_mha (const char* filename, Volume* vol);


/* rpl_volume.cxx */
API void rpl_volume_compute (
        Rpl_volume *rpl_vol,   /* I/O: this gets filled in with depth info */
        Volume *ct_vol         /* I:   the ct volume */
);
API Rpl_volume* rpl_volume_create (
        Volume* ct_vol,       // ct volume
        Proj_matrix *pmat,    // projection matrix from source to aperture
        int ires[2],          // aperture dimensions
        double cam[3],        // position of source
        double ap_ul_room[3], // position of aperture in room coords
        double incr_r[3],     // change in room coordinates for each ap pixel
        double incr_c[3],     // change in room coordinates for each ap pixel
        float ray_step        // uniform ray step size
);
API void rpl_volume_destroy (Rpl_volume *rpl_vol);
API double rpl_volume_get_rgdepth (
        Rpl_volume *rpl_vol,   /* I: volume of radiological depths */
        double* ct_xyz         /* I: location of voxel in world space */
);
API void rpl_volume_save (Rpl_volume *rpl_vol, char *filename);

/* vf_stats.cxx */
API void vf_analyze (Volume* vol);
API void vf_analyze_strain (Volume* vol);
API void vf_analyze_jacobian (Volume* vol);
API void vf_analyze_second_deriv (Volume* vol);
API void vf_analyze_mask (Volume* vol, Volume* mask);
API void vf_analyze_strain_mask (Volume* vol, Volume* mask);
API void vf_print_stats (Volume* vol);

/* volume.cxx */
API void vf_convert_to_interleaved (Volume* ref);
API void vf_convert_to_planar (Volume* ref, int min_size);
API void vf_pad_planar (Volume* vol, int size);  // deprecated?
API Volume* volume_clone_empty (Volume* ref);
API Volume* volume_clone (Volume* ref);
API void volume_convert_to_float (Volume* ref);
API void volume_convert_to_int32 (Volume* ref);
API void volume_convert_to_short (Volume* ref);
API void volume_convert_to_uchar (Volume* ref);
API void volume_convert_to_uint16 (Volume* ref);
API void volume_convert_to_uint32 (Volume* ref);
API Volume* volume_difference (Volume* vol, Volume* warped);
API Volume* volume_make_gradient (Volume* ref);
API void volume_matrix3x3inverse (float *out, const float *m);
API void volume_scale (Volume *vol, float scale);
API Volume* volume_warp (Volume* vout, Volume* vin, Volume* vf);
API void directions_cosine_debug (float *m);

/* volume_limit.h */
API int volume_limit_clip_ray (
        Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
        double *ip1,                /* OUTPUT: Intersection point 1 */
        double *ip2,                /* OUTPUT: Intersection point 2 */
        double *p1,                 /* INPUT:  Starting point of ray */
        double *ray                 /* INPUT:  Direction of ray */
);
API int volume_limit_clip_segment (
        Volume_limit *vol_limit,    /* INPUT:  The bounding box to clip to */
        double *ip1,                /* OUTPUT: Intersection point 1 */
        double *ip2,                /* OUTPUT: Intersection point 2 */
        double *p1,                 /* INPUT:  Line segment point 1 */
        double *p2                  /* INPUT:  Line segment point 2 */
);
API void volume_limit_set (Volume_limit *vol_limit, Volume *vol);

/* volume_resample.h */
XAPI Volume* volume_resample (
        Volume* vol_in,
        const plm_long* dim,
        const float* offset,
        const float* spacing
);
XAPI Volume* volume_resample (Volume* vol_in, const Volume_header *vh);
API Volume* volume_resample_nn (
        Volume* vol_in,
        const plm_long* dim,
        const float* offset,
        const float* spacing
);
API Volume* volume_subsample (Volume* vol_in, int* sampling_rate);
API Volume* volume_subsample_nn (Volume* vol_in, int* sampling_rate);



/* vf_convolve.cxx */
API void vf_convolve_x (Volume* vf_out, Volume* vf_in, float* ker, int width);
API void vf_convolve_y (Volume* vf_out, Volume* vf_in, float* ker, int width);
API void vf_convolve_z (Volume* vf_out, Volume* vf_in, float* ker, int width);

/* vf.cxx */
API Volume* vf_warp (Volume* vout, Volume* vin, Volume* vf); 

/* xpm.cxx */
API xpm_struct* xpm_create(int width, int height, int cpp);
API void xpm_destroy (xpm_struct* xpm);
API void xpm_prime_canvas(xpm_struct* xpm, char color_code);
API void xpm_add_color(xpm_struct* xpm, char color_code, int color);
API int xpm_remove_color(xpm_struct* xpm, char color_code);
API int xpm_draw (xpm_struct* xpm, xpm_brush* brush);
API void xpm_write (xpm_struct* xpm, char* xpm_file);
API xpm_brush* xpm_brush_create ();
API void xpm_brush_destroy (xpm_brush *brush);
API void xpm_brush_set_type (xpm_brush* brush, xpm_brushes type);
API void xpm_brush_set_color (xpm_brush* brush, char color);
API void xpm_brush_set_pos (xpm_brush *brush, int x, int y);
API void xpm_brush_dec_x_pos (xpm_brush *brush, int x);
API void xpm_brush_dec_y_pos (xpm_brush *brush, int y);
API void xpm_brush_inc_x_pos (xpm_brush *brush, int x);
API void xpm_brush_inc_y_pos (xpm_brush *brush, int y);
API void xpm_brush_set_x_pos (xpm_brush *brush, int x);
API void xpm_brush_set_y_pos (xpm_brush *brush, int y);
API void xpm_brush_set_width (xpm_brush* brush, int width);
API void xpm_brush_set_height (xpm_brush* brush, int height);

/* interpolate.cxx */
API void li_clamp (
        float ma,
        plm_long dmax,
        plm_long* maf,
        plm_long* mar, 
        float* fa1, float* fa2
);
API void li_clamp_3d (
        float mijk[3],
        plm_long mijk_f[3],
        plm_long mijk_r[3],
        float li_frac_1[3],
        float li_frac_2[3],
        Volume *moving
);
API float li_value (
        float fx1, float fx2,
        float fy1, float fy2, 
        float fz1, float fz2,
        plm_long mvf, 
        float *m_img,
        Volume *moving
);

#endif
