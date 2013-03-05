/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _hausdorff_distance_h_
#define _hausdorff_distance_h_

#include "plmutil_config.h"
#include "itk_image.h"

class Plm_image;
class Hausdorff_distance_private;

/*! \brief 
 * The Hausdorff class computes the worst-case distance between 
 * two regions.
 *
 * If the images do not have the same size and resolution, the compare 
 * image will be resampled onto the reference image geometry prior 
 * to comparison.  
 */
class PLMUTIL_API Hausdorff_distance {
public:
    Hausdorff_distance ();
    ~Hausdorff_distance ();
public:
    Hausdorff_distance_private *d_ptr;
public:

    /*! \name Inputs */
    ///@{
    /*! \brief Set the reference image.  The image will be loaded
      from the specified filename. */
    void set_reference_image (const char* image_fn);
    /*! \brief Set the reference image as an ITK image. */
    void set_reference_image (const UCharImageType::Pointer image);
    /*! \brief Set the compare image.  The image will be loaded
      from the specified filename. */
    void set_compare_image (const char* image_fn);
    /*! \brief Set the compare image as an ITK image. */
    void set_compare_image (const UCharImageType::Pointer image);
    /*! \brief Set the fraction of voxels to include when computing 
      the percent hausdorff distance.  The input value should be 
      a number between 0 and 1.  The default value is 0.95. */
    void set_hausdorff_distance_fraction (
        float hausdorff_distance_fraction);
    ///@}

    /*! \name Execution */
    ///@{
    /*! \brief Compute hausdorff distances */
    void run ();
    /*! \brief Compute hausdorff distances (obsolete version, doesn't 
      compute 95% Hausdorff) */
    void run_obsolete ();
    ///@}

    /*! \name Outputs */
    ///@{
    /*! \brief Return the Hausdorff distance */
    float get_hausdorff ();
    /*! \brief Return the average Hausdorff distance */
    float get_average_hausdorff ();
    /*! \brief Return the percent Hausdorff distance */
    float get_percent_hausdorff ();
    /*! \brief Return the boundary Hausdorff distance */
    float get_boundary_hausdorff ();
    /*! \brief Return the average boundary Hausdorff distance */
    float get_average_boundary_hausdorff ();
    /*! \brief Return the percent boundary Hausdorff distance */
    float get_percent_boundary_hausdorff ();
    /*! \brief Display debugging information to stdout */
    void debug ();
    ///@}

protected:
    void run_internal (
        UCharImageType::Pointer image1,
        UCharImageType::Pointer image2);
};

PLMUTIL_API
void do_hausdorff(
    UCharImageType::Pointer image_1, 
    UCharImageType::Pointer image_2);

#endif
