/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <stdlib.h>
#include <stdio.h>

#define HAVE_CONFIG_H 1               /* Needed for debian */
#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofstream.h"
#include "dcmtk/dcmdata/dctk.h"

#include "dcmtk_file.h"
#include "print_and_exit.h"

Dcmtk_file::Dcmtk_file () {
}

Dcmtk_file::Dcmtk_file (const char *fn) {
    this->load (fn);
}

Dcmtk_file::~Dcmtk_file () {
}

void
Dcmtk_file::load (const char *fn) {
    DcmFileFormat dfile;
    OFCondition cond = dfile.loadFile (fn, EXS_Unknown, EGL_noChange);
    if (cond.bad()) {
	print_and_exit ("Sorry, couldn't open file as dicom: %s\n", fn);
    }
    DcmDataset *dset = dfile.getDataset();
    dset = dfile.getDataset();

    const char *c = NULL;
    if (dset->findAndGetString(DCM_PatientName, c).good() && c) {
	printf ("Patient name = %s\n", c);
    }    
}

void
dcmtk_file_test (const char *fn)
{
    Dcmtk_file df(fn);
}
