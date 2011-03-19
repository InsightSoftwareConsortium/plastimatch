/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _cxt_to_mha_h_
#define _cxt_to_mha_h_

#include "plm_config.h"
#include "cxt_io.h"
#include "itk_image.h"
#include "plm_image_header.h"
#include "volume.h"

class Cxt_to_mha_state {
public:
    bool want_prefix_imgs;
    bool want_labelmap;
    bool want_ss_img;

    float origin[3];
    float spacing[3];
    int dim[3];

    unsigned char* acc_img;
    Volume* uchar_vol;
    Volume* labelmap_vol;
#if (PLM_USE_SS_IMAGE_VEC)
    UCharVecImageType::Pointer m_ss_img;
#else
    Volume* ss_img_vol;
#endif

    int curr_struct_no;
    int curr_bit;
};

#if defined __cplusplus
extern "C" {
#endif

plastimatch1_EXPORT
void
cxt_to_mha_init (
    Cxt_to_mha_state *ctm_state,
    Rtss_polyline_set *cxt,
    Plm_image_header *pih,
    bool want_prefix_imgs,
    bool want_labelmap,
    bool want_ss_img
);
plastimatch1_EXPORT
bool
cxt_to_mha_process_next (
    Cxt_to_mha_state *ctm_state,
    Rtss_polyline_set *cxt
);
plastimatch1_EXPORT
const char*
cxt_to_mha_current_name (
    Cxt_to_mha_state *ctm_state,
    Rtss_polyline_set *cxt
);
plastimatch1_EXPORT
Cxt_to_mha_state*
cxt_to_mha_create (
    Rtss_polyline_set *cxt,
    Plm_image_header *pih
);
plastimatch1_EXPORT
void
cxt_to_mha_free (Cxt_to_mha_state *ctm_state);
plastimatch1_EXPORT
void
cxt_to_mha_destroy (Cxt_to_mha_state *ctm_state);

#if defined __cplusplus
}
#endif

#endif
