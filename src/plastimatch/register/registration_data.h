/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _registration_data_h_
#define _registration_data_h_

#include "plmregister_config.h"
#include "itk_image_type.h"
#include "plm_image.h"
#include "pointset.h"
#include "registration_parms.h"
#include "smart_pointer.h"

class Plm_image;
class Registration_data_private;
class Shared_parms;
class Stage_parms;

/*! \brief 
 * The Registration_data class holds global data shared across multiple 
 * registration stages.  These data include images, landmarks, 
 * ROIs, and automatic parameters.
 */
class PLMREGISTER_API Registration_data {
public:
    SMART_POINTER_SUPPORT (Registration_data);
    Registration_data_private *d_ptr;
public:
    /* Input images */
    std::map<std::string,Plm_image::Pointer> fixed_image;
    std::map<std::string,Plm_image::Pointer> moving_image;
    Plm_image::Pointer fixed_roi;
    Plm_image::Pointer moving_roi;
    Plm_image::Pointer fixed_stiffness;

    /* Input landmarks */
    Labeled_pointset *fixed_landmarks;
    Labeled_pointset *moving_landmarks;

    /* Region of interest */
    FloatImageType::RegionType fixed_region;
    FloatImageType::PointType fixed_region_origin;
    FloatImageType::SpacingType fixed_region_spacing;

public:
    Registration_data ();
    ~Registration_data ();
    void load_global_input_files (Registration_parms::Pointer& regp);
    void load_stage_input_files (const Stage_parms* regp);
    void load_shared_input_files (const Shared_parms* shared);

    void set_fixed_image (
        const Plm_image::Pointer& image);
    void set_fixed_image (
        const std::string& index,
        const Plm_image::Pointer& image);
    void set_moving_image (
        const Plm_image::Pointer& image);
    void set_moving_image (
        const std::string& index,
        const Plm_image::Pointer& image);
    Plm_image::Pointer& default_fixed_image ();
    Plm_image::Pointer& default_moving_image ();

    Stage_parms* get_auto_parms ();
};

#endif
