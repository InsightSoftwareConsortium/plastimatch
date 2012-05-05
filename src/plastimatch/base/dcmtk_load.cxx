/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"

#include "dcmtk_load.h"
#include "dcmtk_series_set.h"

ShortImageType::Pointer 
dcmtk_load (const char *dicom_dir)
{
    ShortImageType::Pointer img = ShortImageType::New ();
    
    return img;
}

void
dcmtk_rtds_load (Rtds *rtds, const char *dicom_dir)
{
    Dcmtk_series_set dss (dicom_dir);
    dss.load_rtds (rtds);
}
