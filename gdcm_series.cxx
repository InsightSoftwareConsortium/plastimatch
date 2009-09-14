/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdio.h>
#include <stdlib.h>
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "gdcmFile.h"
#include "gdcmFileHelper.h"
#include "gdcmGlobal.h"
#include "gdcmSeqEntry.h"
#include "gdcmSQItem.h"
#include "gdcmUtil.h"
#include "gdcm_series.h"
#include "gdcm_series_helper_2.h"
#include "plm_uid_prefix.h"
#include "plm_version.h"
#include "print_and_exit.h"
#include "readcxt.h"

plastimatch1_EXPORT
void
gdcm_series_test (char *dicom_dir)
{
    Gdcm_series gs;

    gs.load (dicom_dir);
}

static void
print_series_ipp (gdcm::FileList *file_list)
{
    // For all the files of a SingleSerieUID File set
    for (gdcm::FileList::iterator it =  file_list->begin();
	    it != file_list->end(); 
	    ++it)
    {
        double ipp[3];
	ipp[0] = (*it)->GetXOrigin();
	ipp[1] = (*it)->GetYOrigin();
	ipp[2] = (*it)->GetZOrigin();
	printf ("(%g,%g,%g)\t", ipp[0], ipp[1], ipp[2]);
	//printf ("Name = %s\n", (*it)->GetFileName().c_str());
    }
    printf ("\n");
}

static void
digest_file_list (gdcm::FileList *file_list, 
		  double origin[3], 
		  int dim[3], 
		  double spacing[3])
{
    int loop = 0;
    double prev_z;

    // For all the files of a SingleSerieUID File set
    for (gdcm::FileList::iterator it =  file_list->begin();
	    it != file_list->end(); 
	    ++it)
    {
	if (loop == 0) {
	    spacing[0] = (*it)->GetXSpacing ();
	    spacing[1] = (*it)->GetYSpacing ();
	    origin[0] = (*it)->GetXOrigin ();
	    origin[1] = (*it)->GetYOrigin ();
	    prev_z = origin[2] = (*it)->GetZOrigin ();
	    dim[0] = (*it)->GetXSize ();
	    dim[1] = (*it)->GetYSize ();
	    loop ++;
	} else if (loop == 1) {
	    double z = (*it)->GetZOrigin ();
	    if (z - prev_z > 1e-5) {
		spacing[2] = z - origin[2];
		loop ++;
	    } else {
		printf ("Warning: duplicate slice locations (%g)\n", z);
	    }
	    prev_z = z;
	} else {
	    double z = (*it)->GetZOrigin ();
	    if (z - prev_z > 1e-5) {
		//printf (">> %g %g %g\n", z, prev_z, spacing[2]);
		/* XiO rounds IPP to nearest .1 mm */
		if (fabs (z - prev_z - spacing[2]) > 0.11) {
		    print_and_exit ("Error: irregular slice thickness in dicom series\n");
		}
		loop ++;
	    } else {
		printf ("Warning: duplicate slice locations (%g)\n", z);
	    }
	    prev_z = z;
	}
    }
    dim[2] = loop;
}

Gdcm_series::Gdcm_series (void)
{
    this->m_gsh2 = 0;
}

Gdcm_series::~Gdcm_series (void)
{
    if (this->m_gsh2) {
	delete this->m_gsh2;
    }
}

void
Gdcm_series::load (char *dicom_dir)
{
    bool recursive = false;

    this->m_gsh2 = new gdcm::SerieHelper2();

    this->m_gsh2->Clear ();
    this->m_gsh2->CreateDefaultUniqueSeriesIdentifier ();
    this->m_gsh2->SetUseSeriesDetails (true);
    this->m_gsh2->SetDirectory (dicom_dir, recursive);

    //gdcm_shelper->Print ();


    gdcm::FileList *file_list = this->m_gsh2->GetFirstSingleSerieUIDFileSet ();
    while (file_list) {
	if (file_list->size()) {	
	    this->m_gsh2->OrderFileList (file_list);

#if defined (commentout)
	    /* Choose one file, and print the id */
	    gdcm::File *file = (*file_list)[0];
	    std::string id = this->m_gsh2->
		    CreateUniqueSeriesIdentifier(file).c_str();
	    printf ("id = %s\n", id.c_str());

	    /* Iterate through files, and print the ipp */
	    print_series_ipp (file_list);
#endif
	}
	file_list = this->m_gsh2->GetNextSingleSerieUIDFileSet();
    }

}

void
Gdcm_series::get_best_ct (void)
{
    int d;
    for (d = 0; d < 3; d++) {
	this->m_origin[d] = 0.0;
	this->m_dim[d] = 0;
	this->m_spacing[d] = 0.0;
    }

    if (!this->m_gsh2) {
	return;
    }

    gdcm::FileList *file_list = this->m_gsh2->GetFirstSingleSerieUIDFileSet ();
    while (file_list) {
	if (file_list->size()) {	
	    this->m_gsh2->OrderFileList (file_list);

	    /* Get the USI */
	    gdcm::File *file = (*file_list)[0];
	    std::string id = this->m_gsh2->
		    CreateUniqueSeriesIdentifier(file).c_str();

	    /* Is this a CT? */
	    std::string modality = file->GetEntryValue (0x0008, 0x0060);
	    if (modality == std::string ("CT")) {
		int dim[3];
		double origin[3];
		double spacing[3];
	    
		/* Digest the USI */
		digest_file_list (file_list, origin, dim, spacing);
		printf ("---- %s\n", id.c_str());
		printf ("DIM = %d %d %d\n", dim[0], dim[1], dim[2]);
		printf ("OFF = %g %g %g\n", origin[0], origin[1], origin[2]);
		printf ("SPA = %g %g %g\n", spacing[0], 
			spacing[1], spacing[2]);
		
		/* Pick the CT with the largest dim[2] */
		if (dim[2] > this->m_dim[2]) {
		    for (d = 0; d < 3; d++) {
			this->m_origin[d] = origin[d];
			this->m_dim[d] = dim[d];
			this->m_spacing[d] = spacing[d];
		    }
		}
	    }
	}
	file_list = this->m_gsh2->GetNextSingleSerieUIDFileSet();
    }
}
