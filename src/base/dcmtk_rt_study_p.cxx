/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmbase_config.h"
#include "dcmtk_config.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_rt_study_p.h"
#include "dcmtk_series.h"
#include "dcmtk_slice_data.h"
#include "dcmtk_uid.h"
#include "plm_uid_prefix.h"

Dcmtk_rt_study_private::Dcmtk_rt_study_private ()
{
    DcmDate::getCurrentDate (date_string);
    DcmTime::getCurrentTime (time_string);
    const char * const uid_prefix = PlmUidPrefix::getInstance().get().c_str();
    dcmtk_uid (study_uid, uid_prefix);
    dcmtk_uid (for_uid, uid_prefix);
    dcmtk_uid (ct_series_uid, uid_prefix);
    dcmtk_uid (plan_instance_uid, uid_prefix);
    dcmtk_uid (rtss_instance_uid, uid_prefix);
    dcmtk_uid (rtss_series_uid, uid_prefix);
    dcmtk_uid (dose_series_uid, uid_prefix);
    dcmtk_uid (dose_instance_uid, uid_prefix);
    slice_data = new std::vector<Dcmtk_slice_data>;

    rt_study_metadata = Rt_study_metadata::New ();
    filenames_with_uid = true;
}

Dcmtk_rt_study_private::~Dcmtk_rt_study_private ()
{
    /* Delete list of slices */
    delete slice_data;

    /* Delete Dicom_series objects in map */
    Dcmtk_series_map::iterator it;
    for (it = m_smap.begin(); it != m_smap.end(); ++it) {
        delete (*it).second;
    }
}
