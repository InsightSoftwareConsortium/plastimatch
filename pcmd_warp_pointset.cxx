/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include "pcmd_warp.h"
#include "itk_pointset.h"
#include "xform.h"

void
warp_pointset_main (Warp_parms* parms)
{
    Xform xf;
    //FloatPointSetType::Pointer ps_in = FloatPointSetType::New ();

    //itk_pointset_load (ps_in, (const char*) parms->input_fn);
    //itk_pointset_debug (ps_in);

    Pointset *ps = pointset_load ((const char*) parms->input_fn);

    xform_load (&xf, parms->xf_in_fn);

    FloatPointSetType::Pointer ps_in = itk_float_pointset_from_pointset (ps);
    FloatPointSetType::Pointer ps_out = itk_pointset_warp (ps_in, &xf);

    itk_pointset_debug (ps_out);
}
