/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "plm_config.h"
#include "itkArray.h"
#include "itkResampleImageFilter.h"
#include "itkBSplineResampleImageFunction.h"
#include "xform.h"
#include "plm_registration.h"
#include "resample_mha.h"
#include "print_and_exit.h"
#include "volume.h"
#include "bspline.h"
#include "readmha.h"

void init_itk_bsp_default (Xform *xf);
static void
itk_bsp_set_grid (Xform *xf,
	    const BsplineTransformType::OriginType bsp_origin,
	    const BsplineTransformType::SpacingType bsp_spacing,
	    const BsplineTransformType::RegionType bsp_region);
static void
itk_bsp_set_grid_img (Xform *xf,
	      const OriginType& img_origin, 
	      const SpacingType& img_spacing,
	      const ImageRegionType& img_region,
	      float* grid_spac);
static DeformationFieldType::Pointer xform_gpuit_vf_to_itk_vf (Volume* vf, FloatImageType::Pointer image);
static void load_gpuit_bsp (Xform *xf, char* fn);

/* -----------------------------------------------------------------------
   Utility functions
   ----------------------------------------------------------------------- */
static int
strcmp_alt (const char* s1, const char* s2)
{
  return strncmp (s1, s2, strlen(s2));
}

static int
get_parms (FILE* fp, itk::Array<double>* parms, int num_parms)
{
    float f;
    int r, s;

    s = 0;
    while (r = fscanf (fp, "%f",&f)) {
	(*parms)[s++] = (double) f;
	if (s == num_parms) break;
	if (!fp) break;
    }
    return s;
}

/* -----------------------------------------------------------------------
   Load/save functions
   ----------------------------------------------------------------------- */
void
load_xform (Xform *xf, char* fn)
{
    char buf[1024];
    FILE* fp;

    fp = fopen (fn, "r");
    if (!fp) {
	print_and_exit ("Error: xf_in file %s not found\n", fn);
    }
    if (!fgets(buf,1024,fp)) {
	print_and_exit ("Error reading from xf_in file.\n");
    }

    if (strcmp_alt(buf,"ObjectType = MGH_XFORM_TRANSLATION")==0) {
	TranslationTransformType::Pointer trn = TranslationTransformType::New();
	TranslationTransformType::ParametersType xfp(12);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 3);
	if (num_parms != 3) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    trn->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial translation parms = " << trn << std::endl;
#endif
	}
	xf->set_trn (trn);
	fclose (fp);
    } else if (strcmp_alt(buf,"ObjectType = MGH_XFORM_VERSOR")==0) {
	VersorTransformType::Pointer vrs = VersorTransformType::New();
	VersorTransformType::ParametersType xfp(6);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 6);
	if (num_parms != 6) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    vrs->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial versor parms = " << vrs << std::endl;
#endif
	}
	xf->set_vrs (vrs);
	fclose (fp);
    } else if (strcmp_alt(buf,"ObjectType = MGH_XFORM_AFFINE")==0) {
	AffineTransformType::Pointer aff = AffineTransformType::New();
	AffineTransformType::ParametersType xfp(12);
	int num_parms;

	num_parms = get_parms (fp, &xfp, 12);
	if (num_parms != 12) {
	    print_and_exit ("Wrong number of parameters in xf_in file.\n");
	} else {
	    aff->SetParameters(xfp);
#if defined (commentout)
	    std::cout << "Initial affine parms = " << aff << std::endl;
#endif
	}
	xf->set_aff (aff);
	fclose (fp);
    } else if (strcmp_alt(buf,"ObjectType = MGH_XFORM_BSPLINE")==0) {
	int s[3];
	float p[3];
	BsplineTransformType::RegionType::SizeType bsp_size;
	BsplineTransformType::RegionType bsp_region;
	BsplineTransformType::SpacingType bsp_spacing;
	BsplineTransformType::OriginType bsp_origin;

	/* Create the bspline structure */
	init_itk_bsp_default (xf);

	/* Skip 2 lines */
	fgets(buf,1024,fp);
	fgets(buf,1024,fp);

	/* Load bulk transform, if it exists */
	fgets(buf,1024,fp);
	if (!strncmp ("BulkTransform", buf, strlen("BulkTransform"))) {
	    TranslationTransformType::Pointer trn = TranslationTransformType::New();
	    VersorTransformType::Pointer vrs = VersorTransformType::New();
	    AffineTransformType::Pointer aff = AffineTransformType::New();
	    itk::Array <double> xfp(12);
	    float f;
	    int n, num_parm = 0;
	    char *p = buf + strlen("BulkTransform = ");
	    while (sscanf (p, " %g%n", &f, &n) > 0) {
		if (num_parm>=12) {
		    print_and_exit ("Error loading bulk transform\n");
		}
		xfp[num_parm] = f;
		p += n;
		num_parm++;
	    }
	    if (num_parm == 12) {
		aff->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk affine = " << aff;
#endif
		xf->get_bsp()->SetBulkTransform (aff);
	    } else if (num_parm == 6) {
		vrs->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk versor = " << vrs;
#endif
		xf->get_bsp()->SetBulkTransform (vrs);
	    } else if (num_parm == 3) {
		trn->SetParameters(xfp);
#if defined (commentout)
		std::cout << "Bulk translation = " << trn;
#endif
		xf->get_bsp()->SetBulkTransform (trn);
	    } else {
		print_and_exit ("Error loading bulk transform\n");
	    }
	    fgets(buf,1024,fp);
	}

	/* Load origin, spacing, size */
	if (3 != sscanf(buf,"Offset = %g %g %g",&p[0],&p[1],&p[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_origin[0] = p[0]; bsp_origin[1] = p[1]; bsp_origin[2] = p[2];

	fgets(buf,1024,fp);
	if (3 != sscanf(buf,"ElementSpacing = %g %g %g",&p[0],&p[1],&p[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_spacing[0] = p[0]; bsp_spacing[1] = p[1]; bsp_spacing[2] = p[2];

	fgets(buf,1024,fp);
	if (3 != sscanf(buf,"DimSize = %d %d %d",&s[0],&s[1],&s[2])) {
	    print_and_exit ("Unexpected line in xform_in file.\n");
	}
	bsp_size[0] = s[0]; bsp_size[1] = s[1]; bsp_size[2] = s[2];

#if defined (commentout)
	std::cout << "Offset = " << origin << std::endl;
	std::cout << "Spacing = " << spacing << std::endl;
	std::cout << "Size = " << size << std::endl;
#endif

	fgets(buf,1024,fp);
	if (strcmp_alt (buf, "ElementDataFile = LOCAL")) {
	    print_and_exit ("Error: bspline xf_in failed sanity check\n");
	}

	/* Set the BSpline grid to specified parameters */
	bsp_region.SetSize (bsp_size);
	itk_bsp_set_grid (xf, bsp_origin, bsp_spacing, bsp_region);

	/* Read bspline coefficients from file */
	const unsigned int num_parms = xf->get_bsp()->GetNumberOfParameters();
	BsplineTransformType::ParametersType bsp_coeff;
	bsp_coeff.SetSize (num_parms);
	for (int i = 0; i < num_parms; i++) {
	    float d;
	    if (!fgets(buf,1024,fp)) {
		print_and_exit ("Missing bspline coefficient from xform_in file.\n");
	    }
	    if (1 != sscanf(buf,"%g",&d)) {
		print_and_exit ("Bad bspline parm in xform_in file.\n");
	    }
	    bsp_coeff[i] = d;
	}
	fclose (fp);

	/* Copy into bsp structure */
	xf->get_bsp()->SetParametersByValue (bsp_coeff);

    } else if (strcmp_alt(buf,"MGH_GPUIT_BSP <experimental>")==0) {
	fclose (fp);
	load_gpuit_bsp (xf, fn);
    } else {
	/* Close the file and try again, it is probably a vector field */
	fclose (fp);
	DeformationFieldType::Pointer vf = DeformationFieldType::New();
	vf = load_float_field (fn);
	if (!vf) {
	    print_and_exit ("Unexpected file format for xf_in file.\n");
	}
	xf->set_itk_vf (vf);
    }
}

static void
load_gpuit_bsp (Xform *xf, char* fn)
{
    Xform_GPUIT_Bspline* xgb;
    BSPLINE_Parms* parms;
    BSPLINE_Data* bspd;
    char buf[1024];
    FILE* fp;
    int d, rc;

    fp = fopen (fn, "r");
    if (!fp) return;

    /* Initialize parms */
    xgb = (Xform_GPUIT_Bspline*) malloc (sizeof(Xform_GPUIT_Bspline));
    parms = &xgb->parms;
    bspd = &parms->bspd;
    bspline_default_parms (parms);

    /* Skip first line */
    fgets (buf, 1024, fp);

    /* Read header */
    rc = fscanf (fp, "img_origin = %f %f %f\n", &parms->img_origin[0], &parms->img_origin[1], &parms->img_origin[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (img_origin): %s\n", fn);
    }
    rc = fscanf (fp, "img_spacing = %f %f %f\n", &parms->img_spacing[0], &parms->img_spacing[1], &parms->img_spacing[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (img_spacing): %s\n", fn);
    }
    rc = fscanf (fp, "img_dim = %d %d %d\n", &parms->img_dim[0], &parms->img_dim[1], &parms->img_dim[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (img_dim): %s\n", fn);
    }
    rc = fscanf (fp, "roi_offset = %d %d %d\n", &parms->roi_offset[0], &parms->roi_offset[1], &parms->roi_offset[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (roi_offset): %s\n", fn);
    }
    rc = fscanf (fp, "roi_dim = %d %d %d\n", &parms->roi_dim[0], &parms->roi_dim[1], &parms->roi_dim[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (roi_dim): %s\n", fn);
    }
    rc = fscanf (fp, "vox_per_rgn = %d %d %d\n", &parms->vox_per_rgn[0], &parms->vox_per_rgn[1], &parms->vox_per_rgn[2]);
    if (rc != 3) {
	print_and_exit ("Error parsing input xform (vox_per_rgn): %s\n", fn);
    }

    /* Recompute grid_spac */
    for (d = 0; d < 3; d++) {
	parms->grid_spac[d] = parms->vox_per_rgn[d] * parms->img_spacing[d];
    }

    /* Allocate memory and build LUTs */
    bspline_initialize (parms);

    /* This loads from itk-like planar format */
    {
	int i, j;
	for (i = 0; i < 3; i++) {
	    for (j = 0; j < bspd->num_coeff / 3; j++) {
		rc = fscanf (fp, "%f\n", &bspd->coeff[j*3 + i]);
		if (rc != 1) {
		    print_and_exit ("Error parsing input xform (idx = %d,%d): %s\n", i, j, fn);
		}
	    }
	}
    }

    fclose (fp);

    /* Tell Xform that it is gpuit_bsp */
    xf->set_gpuit_bsp (xgb);
}

void
save_xform_translation (TranslationTransformType::Pointer transform, char* filename)
{
    FILE* fp = fopen (filename,"w");
    if (!fp) {
	printf ("Error: Couldn't open file %s for write\n", filename);
	return;
    }

    fprintf (fp,"ObjectType = MGH_XFORM_TRANSLATION\n");
    for (int i = 0; i < transform->GetNumberOfParameters(); i++) {
	fprintf (fp, "%g\n", transform->GetParameters()[i]);
    }
    fclose (fp);
}

void
save_xform_versor (VersorTransformType::Pointer transform, char* filename)
{
    FILE* fp = fopen (filename,"w");
    if (!fp) {
	printf ("Error: Couldn't open file %s for write\n", filename);
	return;
    }

    fprintf (fp,"ObjectType = MGH_XFORM_VERSOR\n");
    for (int i = 0; i < transform->GetNumberOfParameters(); i++) {
	fprintf (fp, "%g\n", transform->GetParameters()[i]);
    }
    fclose (fp);
}

void
save_xform_affine (AffineTransformType::Pointer transform, char* filename)
{
    FILE* fp = fopen (filename,"w");
    if (!fp) {
	printf ("Error: Couldn't open file %s for write\n", filename);
	return;
    }

    fprintf (fp,"ObjectType = MGH_XFORM_AFFINE\n");
    for (int i = 0; i < transform->GetNumberOfParameters(); i++) {
	fprintf (fp, "%g\n", transform->GetParameters()[i]);
    }
    fclose (fp);
}

void
save_xform_itk_bsp (BsplineTransformType::Pointer transform, char* filename)
{
    FILE* fp = fopen (filename,"w");
    if (!fp) {
	printf ("Error: Couldn't open file %s for write\n", filename);
	return;
    }

    fprintf (fp,
	"ObjectType = MGH_XFORM_BSPLINE\n"
	"NDims = 3\n"
	"BinaryData = False\n");
    if (transform->GetBulkTransform()) {
		if ((!strcmp("TranslationTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
			(!strcmp("AffineTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
			(!strcmp("VersorTransform", transform->GetBulkTransform()->GetNameOfClass())) ||
			(!strcmp("VersorRigid3DTransform", transform->GetBulkTransform()->GetNameOfClass()))) {
			fprintf (fp, "BulkTransform =");
			for (int i = 0; i < transform->GetBulkTransform()->GetNumberOfParameters(); i++) {
				fprintf (fp, " %g", transform->GetBulkTransform()->GetParameters()[i]);
			}
			fprintf (fp, "\n");
		} else if (strcmp("IdentityTransform", transform->GetBulkTransform()->GetNameOfClass())) {
			printf("Warning!!! BulkTransform exists. Type=%s\n", transform->GetBulkTransform()->GetNameOfClass());
			printf(" # of parameters=%d\n", transform->GetBulkTransform()->GetNumberOfParameters());
			printf(" The code currently does not know how to handle this type and will not write the parameters out!\n");
		}
    }

    fprintf (fp,
	"Offset = %f %f %f\n"
	"ElementSpacing = %f %f %f\n"
	"DimSize = %d %d %d\n"
	"ElementDataFile = LOCAL\n",
	transform->GetGridOrigin()[0],
	transform->GetGridOrigin()[1],
	transform->GetGridOrigin()[2],
	transform->GetGridSpacing()[0],
	transform->GetGridSpacing()[1],
	transform->GetGridSpacing()[2],
	transform->GetGridRegion().GetSize()[0],
	transform->GetGridRegion().GetSize()[1],
	transform->GetGridRegion().GetSize()[2]
	);
    for (int i = 0; i < transform->GetNumberOfParameters(); i++) {
	fprintf (fp, "%g\n", transform->GetParameters()[i]);
    }
    fclose (fp);
}

void
save_xform (Xform *xf, char* fn)
{
    switch (xf->m_type) {
    case XFORM_ITK_TRANSLATION:
	save_xform_translation (xf->get_trn(), fn);
	break;
    case XFORM_ITK_VERSOR:
	save_xform_versor (xf->get_vrs(), fn);
	break;
    case XFORM_ITK_AFFINE:
	save_xform_affine (xf->get_aff(), fn);
	break;
    case XFORM_ITK_BSPLINE:
	save_xform_itk_bsp (xf->get_bsp(), fn);
	break;
    case XFORM_ITK_VECTOR_FIELD:
	save_image (xf->get_itk_vf(), fn);
	break;
    case XFORM_GPUIT_BSPLINE:
	write_bspd (fn, &xf->get_gpuit_bsp()->parms);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	write_mha (fn, xf->get_gpuit_vf());
	break;
    }
}

#if defined (GCS_REARRANGING_STUFF)
void
init_versor_moments (RegistrationType::Pointer registration,
		       VersorTransformType* versor)
{
    typedef itk::CenteredTransformInitializer < VersorTransformType,
	FloatImageType, FloatImageType > TransformInitializerType;
    TransformInitializerType::Pointer initializer =
	TransformInitializerType::New();

    initializer->SetTransform(versor);
    initializer->SetFixedImage(registration->GetFixedImage());
    initializer->SetMovingImage(registration->GetMovingImage());

    initializer->GeometryOn();

    printf ("Calling Initialize Transform\n");
    initializer->InitializeTransform();

    std::cout << "Transform is " << registration->GetTransform()->GetParameters() << std::endl;
}
#endif /* GCS_REARRANGING_STUFF */

/* -----------------------------------------------------------------------
   Defaults
   ----------------------------------------------------------------------- */
void
init_translation_default (Xform *xf_out, Xform* xf_in, 
		      const OriginType& img_origin, 
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    TranslationTransformType::Pointer trn = TranslationTransformType::New();
    xf_out->set_trn (trn);
}

void
init_versor_default (Xform *xf_out, Xform* xf_in, 
		      const OriginType& img_origin, 
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    VersorTransformType::Pointer vrs = VersorTransformType::New();
    xf_out->set_vrs (vrs);
}

void
init_affine_default (Xform *xf_out, Xform* xf_in, 
		      const OriginType& img_origin, 
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    AffineTransformType::Pointer aff = AffineTransformType::New();
    xf_out->set_aff (aff);
}

void
init_itk_bsp_default (Xform *xf)
{
    BsplineTransformType::Pointer bsp = BsplineTransformType::New();
    xf->set_itk_bsp (bsp);
}

/* -----------------------------------------------------------------------
   Conversion to itk_aff
   ----------------------------------------------------------------------- */
void
xform_trn_to_aff (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    init_affine_default (xf_out, xf_in, img_origin, img_spacing, img_region);
    xf_out->get_aff()->SetOffset(xf_in->get_trn()->GetOffset());
}

void
xform_vrs_to_aff (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    init_affine_default (xf_out, xf_in, img_origin, img_spacing, img_region);
    xf_out->get_aff()->SetMatrix(xf_in->get_vrs()->GetRotationMatrix());
    xf_out->get_aff()->SetOffset(xf_in->get_vrs()->GetOffset());
}

/* -----------------------------------------------------------------------
   Conversion to itk_bsp
   ----------------------------------------------------------------------- */
#if defined (commentout)
static void
init_itk_bsp_region (BsplineTransformType::Pointer bsp,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;
    BsplineTransformType::RegionType::SizeType bsp_size;

    ImageRegionType::SizeType img_size = img_region.GetSize();
    ImageRegionType::IndexType img_index = img_region.GetIndex();

    for (int d=0; d<3; d++) {
	float img_ext = (img_size[d] - 1) * img_spacing[d];
	bsp_origin[d] = img_origin[d] - grid_spac[d];
	bsp_spacing[d] = grid_spac[d];
	bsp_size[d] = 4 + (int) floor (img_ext / grid_spac[d]);
    }

    bsp_region.SetSize (bsp_size);
    bsp->SetGridSpacing (bsp_spacing);
    bsp->SetGridOrigin (bsp_origin);
    bsp->SetGridRegion (bsp_region);

    std::cout << "BSpline Region = "
	      << bsp_region;
    std::cout << "BSpline Grid Origin = "
	      << bsp->GetGridOrigin()
	      << std::endl;
    std::cout << "BSpline Grid Spacing = "
	      << bsp->GetGridSpacing()
	      << std::endl;
    std::cout << "Image Origin = "
	      << img_origin
	      << std::endl;
    std::cout << "Image Spacing = "
	      << img_spacing
	      << std::endl;
    std::cout << "Image Index = "
	      << img_index
	      << std::endl;
    std::cout << "Image Size = "
	      << img_size
	      << std::endl;
}

static void
alloc_itk_bsp_parms (Xform *xf, BsplineTransformType::Pointer bsp)
{
    const unsigned int num_parms = bsp->GetNumberOfParameters();

    xf->m_itk_bsp_parms.SetSize (num_parms);
    xf->m_itk_bsp_parms.Fill (0.0);
    bsp->SetParameters (xf->m_itk_bsp_parms);
    xf->set_itk_bsp (bsp);
}

static void
init_itk_bsp_default (Xform *xf_out, Xform* xf_in, 
		      const OriginType& img_origin, 
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    printf ("init_itk_bsp_default (enter)\n");
    BsplineTransformType::Pointer bsp = BsplineTransformType::New();
    init_itk_bsp_region (bsp, img_origin, img_spacing, img_region, grid_spac);
    alloc_itk_bsp_parms (xf_out, bsp);
    printf ("init_itk_bsp_default (exit)\n");
}
#endif

/* Initialize using bspline spacing */
static void
itk_bsp_set_grid (Xform *xf,
	    const BsplineTransformType::OriginType bsp_origin,
	    const BsplineTransformType::SpacingType bsp_spacing,
	    const BsplineTransformType::RegionType bsp_region)
{
#if defined (commentout)
    printf ("Setting bsp_spacing\n");
    std::cout << bsp_spacing << std::endl;
    printf ("Setting bsp_origin\n");
    std::cout << bsp_origin << std::endl;
    printf ("Setting bsp_region\n");
    std::cout << bsp_region;
#endif

    /* Set grid specifications to bsp struct */
    xf->get_bsp()->SetGridSpacing (bsp_spacing);
    xf->get_bsp()->SetGridOrigin (bsp_origin);
    xf->get_bsp()->SetGridRegion (bsp_region);
    xf->get_bsp()->SetIdentity ();

    /* SetGridRegion automatically initializes internal coefficients to zero */
}

/* Initialize using image spacing */
static void
itk_bsp_set_grid_img (Xform *xf,
	      const OriginType& img_origin, 
	      const SpacingType& img_spacing,
	      const ImageRegionType& img_region,
	      float* grid_spac)
{
    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;
    BsplineTransformType::RegionType::SizeType bsp_size;

    /* Convert image specifications to grid specifications */
    ImageRegionType::SizeType img_size = img_region.GetSize();
    ImageRegionType::IndexType img_index = img_region.GetIndex();
    for (int d=0; d<3; d++) {
	float img_ext = (img_size[d] - 1) * img_spacing[d];
	bsp_origin[d] = img_origin[d] - grid_spac[d];
	bsp_spacing[d] = grid_spac[d];
	bsp_size[d] = 4 + (int) floor (img_ext / grid_spac[d]);
    }
    bsp_region.SetSize (bsp_size);

    /* Finish initialization using grid specifications */
    itk_bsp_set_grid (xf, bsp_origin, bsp_spacing, bsp_region);
}

static void
xform_trn_to_itk_bsp (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    init_itk_bsp_default (xf_out);
    itk_bsp_set_grid_img (xf_out, img_origin, img_spacing, img_region, grid_spac);
    xf_out->get_bsp()->SetBulkTransform (xf_in->get_trn());
}

static void
xform_vrs_to_itk_bsp (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    init_itk_bsp_default (xf_out);
    itk_bsp_set_grid_img (xf_out, img_origin, img_spacing, img_region, grid_spac);
    xf_out->get_bsp()->SetBulkTransform (xf_in->get_vrs());
}

static void
xform_aff_to_itk_bsp (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    init_itk_bsp_default (xf_out);
    itk_bsp_set_grid_img (xf_out, img_origin, img_spacing, img_region, grid_spac);
    xf_out->get_bsp()->SetBulkTransform (xf_in->get_aff());
}

/* GCS Jun 3, 2008.  When going from a lower image resolution to a 
    higher image resolution, the origin pixel moves outside of the 
    valid region defined by the bspline grid.  To fix this, we re-form 
    the b-spline with extra coefficients such that all pixels fall 
    within the valid region. */
void
itk_bsp_extend_to_region (Xform* xf,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region)
{
    int d, i, j, k, old_idx;
    int extend_needed = 0;
    BsplineTransformType::Pointer bsp = xf->get_bsp();
    BsplineTransformType::OriginType bsp_origin = bsp->GetGridOrigin();
    BsplineTransformType::RegionType bsp_region = bsp->GetGridRegion();
    BsplineTransformType::RegionType::SizeType bsp_size = bsp->GetGridRegion().GetSize();
    int eb[3], ea[3];  /* # of control points to "extend before" and "extend after" existing grid */

    /* Figure out if we need to extend the bspline grid.  If so, compute ea & eb, 
       as well as new values of bsp_region and bsp_origin. */
    for (d = 0; d < 3; d++) {
	float old_roi_origin = bsp->GetGridOrigin()[d] + bsp->GetGridSpacing()[d];
	float old_roi_corner = old_roi_origin + (bsp->GetGridRegion().GetSize()[d] - 3) * bsp->GetGridSpacing()[d];
	float new_roi_origin = img_origin[d] + img_region.GetIndex()[d] * img_spacing[d];
	float new_roi_corner = new_roi_origin + (img_region.GetSize()[d] - 1) * img_spacing[d];
#if defined (commentout)
	printf ("extend? [%d]: (%g,%g) -> (%g,%g)\n", d,
		old_roi_origin, old_roi_corner, new_roi_origin, new_roi_corner);
	printf ("img_siz = %d, img_spac = %g\n", img_region.GetSize()[d], img_spacing[d]);
#endif
	ea[d] = eb[d] = 0;
	if (old_roi_origin > new_roi_origin) {
	    float diff = old_roi_origin - new_roi_origin;
	    eb[d] = (int) ceil (diff / bsp->GetGridSpacing()[d]);
	    bsp_origin[d] -= eb[d] * bsp->GetGridSpacing()[d];
	    bsp_size[d] += eb[d];
	    extend_needed = 1;
	}
	if (old_roi_corner < new_roi_corner) {
	    float diff = new_roi_origin - old_roi_origin;
	    ea[d] = (int) ceil (diff / bsp->GetGridSpacing()[d]);
	    bsp_size[d] += ea[d];
	    extend_needed = 1;
	}
    }
#if defined (commentout)
    printf ("ea = %d %d %d, eb = %d %d %d\n", ea[0], ea[1], ea[2], eb[0], eb[1], eb[2]);
#endif
    if (extend_needed) {

	/* Allocate new parameter array */
	BsplineTransformType::Pointer bsp_new = BsplineTransformType::New();
	BsplineTransformType::RegionType old_region = bsp->GetGridRegion();
	bsp_region.SetSize (bsp_size);
	bsp_new->SetGridOrigin (bsp_origin);
	bsp_new->SetGridRegion (bsp_region);
	bsp_new->SetGridSpacing (bsp->GetGridSpacing());

#if defined (commentout)
	std::cout << "OLD BSpline Region = "
		  << bsp->GetGridRegion();
	std::cout << "NEW BSpline Region = "
		  << bsp_new->GetGridRegion();
	std::cout << "OLD BSpline Grid Origin = "
		  << bsp->GetGridOrigin()
		  << std::endl;
	std::cout << "NEW BSpline Grid Origin = "
		  << bsp_new->GetGridOrigin()
		  << std::endl;
	std::cout << "BSpline Grid Spacing = "
		  << bsp_new->GetGridSpacing()
		  << std::endl;
#endif

	/* Copy current parameters in... */
	const unsigned int num_parms = bsp_new->GetNumberOfParameters();
	BsplineTransformType::ParametersType bsp_coeff;
	bsp_coeff.SetSize (num_parms);
	for (old_idx = 0, d = 0; d < 3; d++) {
	    for (k = 0; k < old_region.GetSize()[2]; k++) {
		for (j = 0; j < old_region.GetSize()[1]; j++) {
		    for (i = 0; i < old_region.GetSize()[0]; i++, old_idx++) {
			int new_idx;
			new_idx = ((((d * bsp_size[2]) + k + eb[2]) * bsp_size[1] + (j + eb[1])) * bsp_size[0]) + (i + eb[0]);
			/* GCS FIX - use GetParameters() */
//			printf ("[%4d <- %4d] %g\n", new_idx, old_idx, bsp_parms_old[old_idx]);
			bsp_coeff[new_idx] = bsp->GetParameters()[old_idx];
		    }
		}
	    }
	}

	/* Copy coefficients into recently created bsp struct */
	bsp_new->SetParametersByValue (bsp_coeff);

	/* Fixate xf with new bsp */
	xf->set_itk_bsp (bsp_new);
    }
}

static void
xform_itk_bsp_to_itk_bsp (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region,
		      float* grid_spac)
{
    BsplineTransformType::Pointer bsp_old = xf_in->get_bsp();

    init_itk_bsp_default (xf_out);
    itk_bsp_set_grid_img (xf_out, img_origin, img_spacing, img_region, grid_spac);

    /* Need to copy the bulk transform */
    BsplineTransformType::Pointer bsp_new = xf_out->get_bsp();
    bsp_new->SetBulkTransform (bsp_old->GetBulkTransform());

    /* Create temporary array for output coefficients */
    const unsigned int num_parms = xf_out->get_bsp()->GetNumberOfParameters();
    BsplineTransformType::ParametersType bsp_coeff;
    bsp_coeff.SetSize (num_parms);

    /* GCS May 12, 2008.  I feel like the below algorithm suggested 
	by ITK is wrong.  If BSplineResampleImageFunction interpolates the 
	coefficient image, the resulting resampled B-spline will be smoother 
	than the original.  But maybe the algorithm is OK, just a problem with 
	the lack of proper ITK documentation.  Need to test this. */

    // Now we need to initialize the BSpline coefficients of the higher 
    // resolution transform. This is done by first computing the actual 
    // deformation field at the higher resolution from the lower 
    // resolution BSpline coefficients. Then a BSpline decomposition 
    // is done to obtain the BSpline coefficient of the higher 
    // resolution transform.
    unsigned int counter = 0;
    for (unsigned int k = 0; k < Dimension; k++) {
	typedef BsplineTransformType::ImageType ParametersImageType;
	typedef itk::ResampleImageFilter<ParametersImageType, ParametersImageType> ResamplerType;
	ResamplerType::Pointer upsampler = ResamplerType::New();

	typedef itk::BSplineResampleImageFunction<ParametersImageType, double> FunctionType;
	FunctionType::Pointer fptr = FunctionType::New();

	typedef itk::IdentityTransform<double, Dimension> IdentityTransformType;
	IdentityTransformType::Pointer identity = IdentityTransformType::New();

	upsampler->SetInput (bsp_old->GetCoefficientImage()[k]);
	upsampler->SetInterpolator (fptr);
	upsampler->SetTransform (identity);
	upsampler->SetSize (bsp_new->GetGridRegion().GetSize());
	upsampler->SetOutputSpacing (bsp_new->GetGridSpacing());
	upsampler->SetOutputOrigin (bsp_new->GetGridOrigin());

	typedef itk::BSplineDecompositionImageFilter<ParametersImageType, ParametersImageType> DecompositionType;
	DecompositionType::Pointer decomposition = DecompositionType::New();

	decomposition->SetSplineOrder (SplineOrder);
	decomposition->SetInput (upsampler->GetOutput());
	decomposition->Update();

	ParametersImageType::Pointer newCoefficients = decomposition->GetOutput();

	// copy the coefficients into a temporary parameter array
	typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
	Iterator it (newCoefficients, bsp_new->GetGridRegion());
	while (!it.IsAtEnd()) {
	    bsp_coeff[counter++] = it.Get();
	    ++it;
	}
    }

    /* Finally fixate coefficients into recently created bsp structure */
    bsp_new->SetParametersByValue (bsp_coeff);
}

/* Compute itk_bsp grid specifications from gpuit specifications */
static void
gpuit_bsp_grid_to_itk_bsp_grid (
	BsplineTransformType::OriginType& bsp_origin,	    /* Output */
	BsplineTransformType::SpacingType& bsp_spacing,	    /* Output */
	BsplineTransformType::RegionType& bsp_region,  /* Output */
	Xform_GPUIT_Bspline* xgb)			    /* Input */
{
    BsplineTransformType::SizeType bsp_size;

    for (int d = 0; d < Dimension; d++) {
	bsp_size[d] = xgb->parms.bspd.cdims[d];
	bsp_origin[d] = xgb->parms.img_origin[d] + xgb->parms.roi_offset[d] * xgb->parms.img_spacing[d] - 2 * xgb->parms.grid_spac[d];
	bsp_spacing[d] = xgb->parms.grid_spac[d];
    }
    bsp_region.SetSize (bsp_size);
}

/* Compute itk image specifications from gpuit specifications */
static void
gpuit_bsp_img_region_to_itk_bsp_img_region (
	OriginType& img_origin,		/* Output */
	SpacingType& img_spacing,	/* Output */
	ImageRegionType& img_region,	/* Output */
	int* roi_offset, int* roi_dim, 
	float* img_offset, float* pix_spacing)
{
    ImageRegionType::SizeType img_size;
    ImageRegionType::IndexType img_index;

    for (int d = 0; d < Dimension; d++) {
	img_origin[d] = img_offset[d];
	img_spacing[d] = pix_spacing[d];
	img_index[d] = roi_offset[d];
	img_size[d] = roi_dim[d];
    }
    img_region.SetSize (img_size);
    img_region.SetIndex (img_index);
}

static void
gpuit_bsp_to_itk_bsp_raw (Xform *xf_out, Xform* xf_in)
{
    typedef BsplineTransformType::ImageType ParametersImageType;
    typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
    Xform_GPUIT_Bspline* xgb = xf_in->get_gpuit_bsp();
    BSPLINE_Parms* parms = &xgb->parms;
    BSPLINE_Data* bspd = &parms->bspd;

    BsplineTransformType::OriginType bsp_origin;
    BsplineTransformType::SpacingType bsp_spacing;
    BsplineTransformType::RegionType bsp_region;

    /* Convert bspline grid geometry from gpuit to itk */
    gpuit_bsp_grid_to_itk_bsp_grid (bsp_origin, bsp_spacing, bsp_region, xgb);

    /* Create itk bspline structure */
    init_itk_bsp_default (xf_out);
    itk_bsp_set_grid (xf_out, bsp_origin, bsp_spacing, bsp_region);

    /* RMK: bulk transform is Identity (not supported by GPUIT) */

    /* Create temporary array for output coefficients */
    const unsigned int num_parms = xf_out->get_bsp()->GetNumberOfParameters();
    BsplineTransformType::ParametersType bsp_coeff;
    bsp_coeff.SetSize (num_parms);

    /* Copy from GPUIT coefficient array to ITK coefficient array */
    int k = 0;
    for (int d = 0; d < Dimension; d++) {
	for (int i = 0; i < bspd->num_knots; i++) {
	    bsp_coeff[k] = bspd->coeff[3*i+d];
	    k++;
	}
    }

    /* Fixate coefficients into bsp structure */
    xf_out->get_bsp()->SetParametersByValue (bsp_coeff);
}

void
xform_gpuit_bsp_to_itk_bsp (Xform *xf_out, Xform* xf_in,
		      const OriginType& img_origin,
		      const SpacingType& img_spacing,
		      const ImageRegionType& img_region, 
		      float* grid_spac)
{
    typedef BsplineTransformType::ImageType ParametersImageType;
    typedef itk::ImageRegionIterator<ParametersImageType> Iterator;
    Xform_GPUIT_Bspline* xgb = xf_in->get_gpuit_bsp();
    BSPLINE_Parms* parms = &xgb->parms;
    BSPLINE_Data* bspd = &parms->bspd;
    Xform xf_tmp;
    OriginType img_origin_old;
    SpacingType img_spacing_old;
    ImageRegionType img_region_old;

    /* Convert to itk data structure */
    printf ("Running: gpuit_bsp_to_itk_bsp_raw\n");
    gpuit_bsp_to_itk_bsp_raw (&xf_tmp, xf_in);

    /* Then, resample the xform to the desired grid spacing */
    printf ("Running: xform_itk_bsp_to_itk_bsp\n");
    xform_itk_bsp_to_itk_bsp (xf_out, &xf_tmp,
		      img_origin, img_spacing, img_region, grid_spac);
}

/* -----------------------------------------------------------------------
   Conversion to itk_vf
   ----------------------------------------------------------------------- */
static DeformationFieldType::Pointer
xform_any_to_itk_vf (itk::Transform<double,3,3>* xf,
		     const int* dim, float* offset, float* spacing)
{
    int i;
    DeformationFieldType::SizeType sz;
    DeformationFieldType::IndexType st;
    DeformationFieldType::RegionType rg;
    DeformationFieldType::PointType og;
    DeformationFieldType::SpacingType sp;
    DeformationFieldType::Pointer itk_vf = DeformationFieldType::New();

    /* Copy header & allocate data for itk */
    for (i = 0; i < 3; i++) {
	st[i] = 0;
	sz[i] = dim[i];
	sp[i] = spacing[i];
	og[i] = offset[i];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);

    itk_vf->SetRegions (rg);
    itk_vf->SetOrigin (og);
    itk_vf->SetSpacing (sp);
    itk_vf->Allocate ();

    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());

    fi.GoToBegin();

    DoublePointType fixed_point;
    DoublePointType moving_point;
    DeformationFieldType::IndexType index;

    FloatVectorType displacement;

    while (!fi.IsAtEnd()) {
	index = fi.GetIndex();
	itk_vf->TransformIndexToPhysicalPoint (index, fixed_point);
	moving_point = xf->TransformPoint (fixed_point);
	for (int r = 0; r < Dimension; r++) {
	    displacement[r] = moving_point[r] - fixed_point[r];
	}
	fi.Set (displacement);
	++fi;
    }
    return itk_vf;
}

/* Note: function name is overloaded for backward compatibility.  
   Try to rename soon... */
DeformationFieldType::Pointer
xform_any_to_itk_vf (itk::Transform<double,3,3>* xf,
		 FloatImageType::Pointer image)
{
    DeformationFieldType::Pointer field = DeformationFieldType::New();
    field->SetRegions (image->GetLargestPossibleRegion());
    field->SetOrigin (image->GetOrigin());
    field->SetSpacing (image->GetSpacing());
    field->Allocate();

    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (field, image->GetLargestPossibleRegion());

    DoublePointType fixed_point;
    DoublePointType moving_point;
    DeformationFieldType::IndexType index;

    FloatVectorType displacement;

    fi.GoToBegin();
    while (!fi.IsAtEnd()) {
	index = fi.GetIndex();
	field->TransformIndexToPhysicalPoint (index, fixed_point);
	moving_point = xf->TransformPoint (fixed_point);
	for (int r = 0; r < Dimension; r++) {
	    displacement[r] = moving_point[r] - fixed_point[r];
	}
	fi.Set (displacement);
	++fi;
    }
    return field;
}

DeformationFieldType::Pointer 
xform_itk_vf_to_itk_vf (DeformationFieldType::Pointer vf,
	         FloatImageType::Pointer image)
{
    const DeformationFieldType::SpacingType& vf_spacing = vf->GetSpacing();
    const DeformationFieldType::SizeType vf_size = vf->GetLargestPossibleRegion().GetSize();
    const FloatImageType::SizeType& img_size = image->GetLargestPossibleRegion().GetSize();
    const FloatImageType::SpacingType& img_spacing = image->GetSpacing();

    if (vf_size[0] != img_size[0] || vf_size[1] != img_size[1] || vf_size[2] != img_size[2]) {
	vf = vector_resample_image (vf, image);
    }
    return vf;
}

/* Here what we're going to do is use GPUIT library to interpolate the 
    B-Spline at its native resolution, then convert gpuit_vf -> itk_vf. 

    GCS: Aug 6, 2008.  The above idea doesn't work, because the native 
    resolution might not encompass the image.  Here is what we will do:
    1) Convert to ITK B-Spline
    2) Extend ITK B-Spline to encompass image
    3) Render vf.
    */
static DeformationFieldType::Pointer
xform_gpuit_bsp_to_itk_vf (//Xform_GPUIT_Bspline *xgb, 
			    Xform* xf_in,
			    FloatImageType::Pointer image)
{
    //Volume *vf;
    //float* img;
    DeformationFieldType::Pointer itk_vf;

    Xform xf_tmp;
    Xform_GPUIT_Bspline* xgb_old = xf_in->get_gpuit_bsp();
    OriginType img_origin;
    SpacingType img_spacing;
    ImageRegionType img_region;

    /* Copy from GPUIT coefficient array to ITK coefficient array */
    gpuit_bsp_to_itk_bsp_raw (&xf_tmp, xf_in);

    /* Resize itk array to span image */
    itk_bsp_extend_to_region (&xf_tmp, image->GetOrigin(),
	    image->GetSpacing(), image->GetLargestPossibleRegion());

    /* Render to vector field */
    itk_vf = xform_any_to_itk_vf (xf_tmp.get_bsp(), image);

    return itk_vf;
}

/* This is really dumb.  But what we're going to do is 
   to convert gpuit -> itk at the native resolution, 
   then itk -> itk to change resolution.  */
static DeformationFieldType::Pointer 
xform_gpuit_vf_to_itk_vf (Volume* vf,
			    FloatImageType::Pointer image)
{
    int i;
    DeformationFieldType::SizeType sz;
    DeformationFieldType::IndexType st;
    DeformationFieldType::RegionType rg;
    DeformationFieldType::PointType og;
    DeformationFieldType::SpacingType sp;
    DeformationFieldType::Pointer itk_vf = DeformationFieldType::New();
    FloatVectorType displacement;

    /* Copy header & allocate data for itk */
    for (i = 0; i < 3; i++) {
	st[i] = 0;
	sz[i] = vf->dim[i];
	sp[i] = vf->pix_spacing[i];
	og[i] = vf->offset[i];
    }
    rg.SetSize (sz);
    rg.SetIndex (st);

    itk_vf->SetRegions (rg);
    itk_vf->SetOrigin (og);
    itk_vf->SetSpacing (sp);
    itk_vf->Allocate();

    /* Copy data into itk */
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());

    if (vf->pix_type == PT_VF_FLOAT_INTERLEAVED) {
	float* img = (float*) vf->img;
	int i = 0;
	for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
	    for (int r = 0; r < Dimension; r++) {
		displacement[r] = img[i++];
	    }
	    fi.Set (displacement);
	}
    }
    else if (vf->pix_type == PT_VF_FLOAT_PLANAR) {
	float** img = (float**) vf->img;
	int i = 0;
	for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi, ++i) {
	    for (int r = 0; r < Dimension; r++) {
		displacement[r] = img[r][i];
	    }
	    fi.Set (displacement);
	}
    } else {
	print_and_exit ("Irregular pix_type used converting gpuit_xf -> itk\n");
    }

    /* Resample to requested resolution */
    itk_vf = xform_itk_vf_to_itk_vf (itk_vf, image);

    return itk_vf;
}

/* -----------------------------------------------------------------------
   Conversion to gpuit_bsp
   ----------------------------------------------------------------------- */
void
xform_gpuit_bsp_to_gpuit_bsp (Xform* xf_out, Xform* xf_in, Xform_GPUIT_Bspline* xgb_new)
{
    Xform xf_tmp;
    Xform_GPUIT_Bspline* xgb_old = xf_in->get_gpuit_bsp();
    BSPLINE_Parms* parms_old = &xgb_old->parms;
    BSPLINE_Data* bspd_old = &parms_old->bspd;
    BSPLINE_Parms* parms_new = &xgb_new->parms;
    BSPLINE_Data* bspd_new = &parms_new->bspd;
    OriginType img_origin;
    SpacingType img_spacing;
    ImageRegionType img_region;

    /* Figure out image specification, etc. from gpuit structure */
    gpuit_bsp_img_region_to_itk_bsp_img_region (img_origin, 
	    img_spacing, img_region, 
	    parms_new->roi_offset, parms_new->roi_dim, 
	    parms_new->img_origin, parms_new->img_spacing);

    /* Create itk_bsp xf using image specifications */
    xform_gpuit_bsp_to_itk_bsp (&xf_tmp, xf_in, 
	    img_origin, img_spacing, 
	    img_region, parms_new->grid_spac);

    /* Copy from ITK coefficient array to gpuit coefficient array */
    int k = 0;
    for (int d = 0; d < Dimension; d++) {
	for (int i = 0; i < bspd_new->num_knots; i++) {
	    bspd_new->coeff[3*i+d] = xf_tmp.get_bsp()->GetParameters()[k];
	    k++;
	}
    }
}


/* -----------------------------------------------------------------------
   Conversion to gpuit_vf
   ----------------------------------------------------------------------- */
Volume*
xform_gpuit_vf_to_gpuit_vf (Volume* vf_in, int* dim, float* offset, float* pix_spacing)
{
    Volume* vf_out;
    vf_out = volume_resample (vf_in, dim, offset, pix_spacing);
    return vf_out;
}

Volume*
xform_gpuit_bsp_to_gpuit_vf (Xform* xf_in, int* dim, float* offset, float* pix_spacing)
{
    BSPLINE_Parms* bsp = &xf_in->get_gpuit_bsp()->parms;
    Volume* vf_out;

    vf_out = volume_create (dim, offset, pix_spacing, PT_VF_FLOAT_INTERLEAVED, 0);
    bspline_interpolate_vf (vf_out, bsp);
    return vf_out;
}

Volume*
xform_itk_vf_to_gpuit_vf (DeformationFieldType::Pointer itk_vf, int* dim, float* offset, float* pix_spacing)
{
    Volume* vf_out = volume_create (dim, offset, pix_spacing, PT_VF_FLOAT_INTERLEAVED, 0);
    float* img = (float*) vf_out->img;
    FloatVectorType displacement;

    int i = 0;
    typedef itk::ImageRegionIterator< DeformationFieldType > FieldIterator;
    FieldIterator fi (itk_vf, itk_vf->GetLargestPossibleRegion());
    for (fi.GoToBegin(); !fi.IsAtEnd(); ++fi) {
	displacement = fi.Get ();
	for (int r = 0; r < Dimension; r++) {
	    img[i++] = displacement[r];
	}
    }
    return vf_out;
}


/* -----------------------------------------------------------------------
   Selection routines to convert from X to Y
   ----------------------------------------------------------------------- */
void
xform_to_trn (Xform *xf_out, 
	      Xform *xf_in, 
	      const OriginType& img_origin, 
	      const SpacingType& img_spacing,
	      const ImageRegionType& img_region)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
	init_translation_default (xf_out, xf_in, img_origin, img_spacing, img_region);
	break;
    case XFORM_ITK_TRANSLATION:
	*xf_out = *xf_in;
	break;
    case XFORM_ITK_VERSOR:
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, couldn't convert to trn\n");
	break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
}

void
xform_to_vrs (Xform *xf_out, 
	      Xform *xf_in, 
	      const OriginType& img_origin, 
	      const SpacingType& img_spacing,
	      const ImageRegionType& img_region)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
	init_versor_default (xf_out, xf_in, img_origin, img_spacing, img_region);
	break;
    case XFORM_ITK_TRANSLATION:
	print_and_exit ("Sorry, couldn't convert to vrs\n");
	break;
    case XFORM_ITK_VERSOR:
	*xf_out = *xf_in;
	break;
    case XFORM_ITK_AFFINE:
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, couldn't convert to vrs\n");
	break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
}

void
xform_to_aff (Xform *xf_out, 
	      Xform *xf_in, 
	      const OriginType& img_origin, 
	      const SpacingType& img_spacing,
	      const ImageRegionType& img_region)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
	init_affine_default (xf_out, xf_in, img_origin, img_spacing, img_region);
	break;
    case XFORM_ITK_TRANSLATION:
	xform_trn_to_aff (xf_out, xf_in, img_origin, img_spacing, img_region);
	break;
    case XFORM_ITK_VERSOR:
	xform_vrs_to_aff (xf_out, xf_in, img_origin, img_spacing, img_region);
	break;
    case XFORM_ITK_AFFINE:
	*xf_out = *xf_in;
	break;
    case XFORM_ITK_BSPLINE:
    case XFORM_ITK_TPS:
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, couldn't convert to aff\n");
	break;
    case XFORM_GPUIT_BSPLINE:
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, gpuit xforms not fully implemented\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
}

void
xform_to_itk_bsp (Xform *xf_out, 
		  Xform *xf_in, 
		  const OriginType& img_origin, 
		  const SpacingType& img_spacing,
		  const ImageRegionType& img_region,
		  float* grid_spac)
{
    BsplineTransformType::Pointer bsp;

    switch (xf_in->m_type) {
    case XFORM_NONE:
	init_itk_bsp_default (xf_out);
	itk_bsp_set_grid_img (xf_out, img_origin, img_spacing, img_region, grid_spac);
	break;
    case XFORM_ITK_TRANSLATION:
	xform_trn_to_itk_bsp (xf_out, xf_in, 
				img_origin, img_spacing, img_region, grid_spac);
	break;
    case XFORM_ITK_VERSOR:
	xform_vrs_to_itk_bsp (xf_out, xf_in, 
				img_origin, img_spacing, img_region, grid_spac);
	break;
    case XFORM_ITK_AFFINE:
	xform_aff_to_itk_bsp (xf_out, xf_in, 
				img_origin, img_spacing, img_region, grid_spac);
	break;
    case XFORM_ITK_BSPLINE:
	xform_itk_bsp_to_itk_bsp (xf_out, xf_in, 
				img_origin, img_spacing, img_region, 
				grid_spac);
	break;
    case XFORM_ITK_TPS:
	print_and_exit ("Sorry, couldn't convert itk_tps to itk_bsp\n");
	break;
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, couldn't convert itk_vf to itk_bsp\n");
	break;
    case XFORM_GPUIT_BSPLINE:
	xform_gpuit_bsp_to_itk_bsp (xf_out, xf_in, 
				    img_origin, img_spacing, img_region,
				    grid_spac);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, couldn't convert gpuit_vf to itk_bsp\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
}

void
xform_to_itk_vf (Xform* xf_out, Xform *xf_in, const int* dim, float* offset, float* spacing)
{
    DeformationFieldType::Pointer vf;

    switch (xf_in->m_type) {
    case XFORM_NONE:
	print_and_exit ("Sorry, couldn't convert to vf\n");
	break;
    case XFORM_ITK_TRANSLATION:
	vf = xform_any_to_itk_vf (xf_in->get_trn(), dim, offset, spacing);
	break;
    case XFORM_ITK_VERSOR:
	vf = xform_any_to_itk_vf (xf_in->get_vrs(), dim, offset, spacing);
	break;
    case XFORM_ITK_AFFINE:
	vf = xform_any_to_itk_vf (xf_in->get_aff(), dim, offset, spacing);
	break;
    case XFORM_ITK_BSPLINE:
	vf = xform_any_to_itk_vf (xf_in->get_bsp(), dim, offset, spacing);
	break;
    case XFORM_ITK_TPS:
	vf = xform_any_to_itk_vf (xf_in->get_itk_tps(), dim, offset, spacing);
	break;
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, itk_vf to itk_vf not implemented\n");
	break;
    case XFORM_GPUIT_BSPLINE:
	print_and_exit ("Sorry, gpuit_bsp to itk_vf not implemented\n");
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, gpuit_vf to itk_vf not implemented\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
    xf_out->set_itk_vf (vf);
}

/* Overloaded fn.  Fix soon... */
void
xform_to_itk_vf (Xform* xf_out, Xform *xf_in, FloatImageType::Pointer image)
{
    DeformationFieldType::Pointer vf;

    switch (xf_in->m_type) {
    case XFORM_NONE:
	print_and_exit ("Sorry, couldn't convert to vf\n");
	break;
    case XFORM_ITK_TRANSLATION:
	vf = xform_any_to_itk_vf (xf_in->get_trn(), image);
	break;
    case XFORM_ITK_VERSOR:
	vf = xform_any_to_itk_vf (xf_in->get_vrs(), image);
	break;
    case XFORM_ITK_AFFINE:
	vf = xform_any_to_itk_vf (xf_in->get_aff(), image);
	break;
    case XFORM_ITK_BSPLINE:
	/* Why do I need this??? */
	itk_bsp_extend_to_region (xf_in, image->GetOrigin(),
			image->GetSpacing(), image->GetLargestPossibleRegion());
	vf = xform_any_to_itk_vf (xf_in->get_bsp(), image);
	break;
    case XFORM_ITK_TPS:
	vf = xform_any_to_itk_vf (xf_in->get_itk_tps(), image);
	break;
    case XFORM_ITK_VECTOR_FIELD:
	vf = xform_itk_vf_to_itk_vf (xf_in->get_itk_vf(), image);
	break;
    case XFORM_GPUIT_BSPLINE:
	vf = xform_gpuit_bsp_to_itk_vf (xf_in, image);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	vf = xform_gpuit_vf_to_itk_vf (xf_in->get_gpuit_vf(), image);
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
    xf_out->set_itk_vf (vf);
}

void
xform_to_gpuit_bsp (Xform* xf_out, Xform* xf_in, Xform_GPUIT_Bspline* xgb_new)
{
    switch (xf_in->m_type) {
    case XFORM_NONE:
	/* Do nothing */
	break;
    case XFORM_ITK_TRANSLATION:
	print_and_exit ("Sorry, itk_translation to gpuit_bsp not implemented\n");
	break;
    case XFORM_ITK_VERSOR:
	print_and_exit ("Sorry, itk_versor to gpuit_bsp not implemented\n");
	break;
    case XFORM_ITK_AFFINE:
	print_and_exit ("Sorry, itk_affine to gpuit_bsp not implemented\n");
	break;
    case XFORM_ITK_BSPLINE:
	print_and_exit ("Sorry, itk_bsp to gpuit_bsp not implemented\n");
	break;
    case XFORM_ITK_TPS:
	print_and_exit ("Sorry, itk_tps to gpuit_bsp not implemented\n");
	break;
    case XFORM_ITK_VECTOR_FIELD:
	print_and_exit ("Sorry, itk_vf to gpuit_bsp not implemented\n");
	break;
    case XFORM_GPUIT_BSPLINE:
	xform_gpuit_bsp_to_gpuit_bsp (xf_out, xf_in, xgb_new);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	print_and_exit ("Sorry, gpuit_vf to gpuit_bsp not implemented\n");
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }
    xf_out->set_gpuit_bsp (xgb_new);
}

void
xform_to_gpuit_vf (Xform* xf_out, Xform *xf_in, int* dim, float* offset, float* pix_spacing)
{
    Volume* vf = 0;

    switch (xf_in->m_type) {
    case XFORM_NONE:
	print_and_exit ("Sorry, couldn't convert NONE to gpuit_vf\n");
	break;
    case XFORM_ITK_TRANSLATION:
	print_and_exit ("Sorry, itk_translation to gpuit_vf not implemented\n");
	break;
    case XFORM_ITK_VERSOR:
	print_and_exit ("Sorry, itk_versor to gpuit_vf not implemented\n");
	break;
    case XFORM_ITK_AFFINE:
	print_and_exit ("Sorry, itk_affine to gpuit_vf not implemented\n");
	break;
    case XFORM_ITK_BSPLINE:
	print_and_exit ("Sorry, itk_bspline to gpuit_vf not implemented\n");
	break;
    case XFORM_ITK_TPS:
	print_and_exit ("Sorry, itk_tps to gpuit_vf not implemented\n");
	break;
    case XFORM_ITK_VECTOR_FIELD:
	vf = xform_itk_vf_to_gpuit_vf (xf_in->get_itk_vf(), dim, offset, pix_spacing);
	break;
    case XFORM_GPUIT_BSPLINE:
	vf = xform_gpuit_bsp_to_gpuit_vf (xf_in, dim, offset, pix_spacing);
	break;
    case XFORM_GPUIT_VECTOR_FIELD:
	vf = xform_gpuit_vf_to_gpuit_vf (xf_in->get_gpuit_vf(), dim, offset, pix_spacing);
	break;
    default:
	print_and_exit ("Program error.  Bad xform type.\n");
	break;
    }

    xf_out->set_gpuit_vf (vf);
}
