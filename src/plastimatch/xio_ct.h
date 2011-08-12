/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _xio_ct_h_
#define _xio_ct_h_

#include "plm_config.h"
#include "plm_image.h"
#include "xio_studyset.h"

typedef struct xio_ct_transform Xio_ct_transform;
struct xio_ct_transform {
    Plm_image_patient_position patient_pos;
    float direction_cosines[9];
    float x_offset;
    float y_offset;
};

plastimatch1_EXPORT 
void
xio_ct_load (Plm_image *plm, const Xio_studyset *xio_studyset);
plastimatch1_EXPORT 
void
xio_ct_get_transform_from_dicom_dir (
    Plm_image *plm,
    Xio_ct_transform *transform,
    const char *dicom_dir
);
plastimatch1_EXPORT 
void xio_ct_get_transform (Plm_image *plm, Xio_ct_transform *transform);
plastimatch1_EXPORT 
void
xio_ct_apply_transform (Plm_image *plm, Xio_ct_transform *transform);

#endif
