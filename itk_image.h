/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#ifndef _load_mha_h_
#define _load_mha_h_

#include <stdio.h>
#include "itkImage.h"
#include "itkImageIOBase.h"
#include "plm_config.h"

#ifndef _MAX_PATH
#ifdef FILENAME_MAX
#define _MAX_PATH FILENAME_MAX
#else
#define _MAX_PATH 256
#endif
#endif

/* We only deal with these kinds of images. */
#define TYPE_UNSPECIFIED      0
#define TYPE_USHORT           1
#define TYPE_SHORT            2
#define TYPE_FLOAT            3
#define TYPE_UCHAR            4
#define TYPE_FLOAT_FIELD      5

/* We only deal with 3D images. */
const unsigned int Dimension = 3;

typedef itk::Image < unsigned char, Dimension > UCharImageType;
typedef itk::Image < short, Dimension > ShortImageType;
typedef itk::Image < unsigned short, Dimension > UShortImageType;
typedef itk::Image < float, Dimension > FloatImageType;

typedef itk::Image < short, 2 > ShortImage2DType;

typedef itk::Point < float, Dimension > FloatPointType;
typedef itk::Vector < float, Dimension > FloatVectorType;
typedef itk::Image < FloatVectorType, Dimension > DeformationFieldType;

typedef itk::Point < double, Dimension > DoublePointType;
typedef itk::Vector < double, Dimension > DoubleVectorType;

typedef itk::Size < Dimension > SizeType;
typedef itk::Point < double, Dimension >  OriginType;
typedef itk::Vector < double, Dimension > SpacingType;
typedef itk::ImageRegion < Dimension > ImageRegionType;

/* -----------------------------------------------------------------------
   Function prototypes
   ----------------------------------------------------------------------- */
plastimatch1_EXPORT UCharImageType::Pointer load_uchar (char* fname);
plastimatch1_EXPORT ShortImageType::Pointer load_short (char* fname);
plastimatch1_EXPORT FloatImageType::Pointer load_float (char* fname);
plastimatch1_EXPORT DeformationFieldType::Pointer load_float_field (char* fname);

plastimatch1_EXPORT void itk__GetImageType (std::string fileName,
			itk::ImageIOBase::IOPixelType &pixelType,
			itk::ImageIOBase::IOComponentType &componentType);

#if defined (commentout)
plastimatch1_EXPORT void get_image_header (int dim[3], float offset[3], float spacing[3], UCharImageType::Pointer image);
plastimatch1_EXPORT void get_image_header (int dim[3], float offset[3], float spacing[3], ShortImageType::Pointer image);
plastimatch1_EXPORT void get_image_header (int dim[3], float offset[3], float spacing[3], FloatImageType::Pointer image);
#endif
template<class T> void get_image_header (int dim[3], float offset[3], float spacing[3], T image);

template<class T> void save_image (T img_ptr, char* fname);
template<class T> void save_short_dicom (T image, char* dir_name);
template<class T> void save_short (T img_ptr, char* fname);
template<class T> void save_float (T img_ptr, char* fname);

#endif
