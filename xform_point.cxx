/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "xform_point.h"

/* -----------------------------------------------------------------------
   Transform points
   ----------------------------------------------------------------------- */
void
xform_point_transform_itk_vf (
    FloatPointType* point_out, 
    Xform* xf_in, 
    FloatPointType point_in
)
{
    DeformationFieldType::Pointer vf = xf_in->get_itk_vf ();
    DeformationFieldType::IndexType idx;

    bool isInside = vf->TransformPhysicalPointToIndex (point_in, idx);
    if (isInside) {
	DeformationFieldType::PixelType pixelValue = vf->GetPixel (idx);
	printf ("pi [%g %g %g]\n", point_in[0], point_in[1], point_in[2]);
	printf ("idx [%ld %ld %ld]\n", idx[0], idx[1], idx[2]);
	printf ("vf [%g %g %g]\n", pixelValue[0], pixelValue[1], pixelValue[2]);
	for (int d = 0; d < 3; d++) {
	    (*point_out)[d] = point_in[d] + pixelValue[d];
	}
	printf ("po [%g %g %g]\n", 
	    (*point_out)[0], (*point_out)[1], (*point_out)[2]);
    } else {
	(*point_out) = point_in;
    }
}

void
xform_point_transform (
    FloatPointType* point_out, 
    Xform* xf_in, 
    FloatPointType point_in
)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
    case XFORM_ITK_TRANSLATION:
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_QUATERNION:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
	print_and_exit (
	    "Sorry, xform_transform_point not defined for type %d\n",
	    xf_in->m_type);
	break;
    case XFORM_ITK_VECTOR_FIELD:
	xform_point_transform_itk_vf (point_out, xf_in, point_in);
	break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
    default:
	print_and_exit (
	    "Sorry, xform_transform_point not defined for type %d\n",
	    xf_in->m_type);
	break;
    }
}
