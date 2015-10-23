/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bragg_curve.h"
#include "rt_depth_dose.h"

Rt_depth_dose::Rt_depth_dose ()
{
    this->d_lut = NULL;
    this->e_lut = NULL;
	this->f_lut = NULL;

    this->E0 = 100.0;
    this->spread = 1.0;
    this->dres = .01;
    this->dmax = 400.0;
    this->weight = 1.0;

    this->num_samples = 40000;
}

Rt_depth_dose::Rt_depth_dose (
    double E0, double spread, double dres, double dmax, double weight)
{
    this->d_lut = NULL;
    this->e_lut = NULL;
	this->f_lut = NULL;

    this->E0 = E0;
    this->spread = spread;
    this->dres = dres;
    this->dmax = dmax;
    this->weight = weight;

    this->generate();
}

Rt_depth_dose::~Rt_depth_dose ()
{
    if (this->d_lut) {
        free (this->d_lut);
    }
    if (this->e_lut) {
        free (this->e_lut);
    }
	if (this->f_lut) {
		free (this->f_lut);
	}
}

bool
Rt_depth_dose::load (const char* fn)
{
    FILE* fp = fopen (fn, "r");
    char linebuf[128];

    if (!fp) {
        return false;
    }

    fgets (linebuf, 128, fp);
    fclose (fp);

    if (!strncmp (linebuf, "00001037", strlen ("00001037"))) {
        return this->load_xio (fn);
    } else {
        return this->load_txt (fn);
    }
}

bool
Rt_depth_dose::load_xio (const char* fn)
{
    int i, j;
    char* ptoken;
    char linebuf[128];
    FILE* fp = fopen (fn, "r");

    // Need to check for a magic number (00001037) here?
    
    /* skip the first 4 lines */
    for (i=0; i<4; i++) {
        fgets (linebuf, 128, fp);
    }

    /* line 5 contains the # of samples */
    fgets (linebuf, 128, fp);
    sscanf (linebuf, "%i", &this->num_samples);

    this->d_lut = (float*)malloc (this->num_samples*sizeof(float));
    this->e_lut = (float*)malloc (this->num_samples*sizeof(float));
	this->f_lut = (float*)malloc (this->num_samples*sizeof(float));
    
    memset (this->d_lut, 0, this->num_samples*sizeof(float));
    memset (this->e_lut, 0, this->num_samples*sizeof(float));
	memset (this->f_lut, 0, this->num_samples*sizeof(float));

    /* load in the depths (10 samples per line) */
    for (i=0, j=0; i<(this->num_samples/10)+1; i++) {
        fgets (linebuf, 128, fp);
        ptoken = strtok (linebuf, ",\n\0");
        while (ptoken) {
            this->d_lut[j++] = (float) strtod (ptoken, NULL);
            ptoken = strtok (NULL, ",\n\0");
        }
    }
    this->dmax = this->d_lut[j-1];

    /* load in the energies (10 samples per line) */
    for (i=0, j=0; i<(this->num_samples/10)+1; i++) {
        fgets (linebuf, 128, fp);
        ptoken = strtok (linebuf, ",\n\0");
        while (ptoken) {
            this->e_lut[j] = (float) strtod (ptoken, NULL);
            ptoken = strtok (NULL, ",\n\0");
            j++;
        }
    }

	/* load in the energies (10 samples per line) */
    for (i=0, j=0; i<(this->num_samples/10)+1; i++) {
        fgets (linebuf, 128, fp);
        ptoken = strtok (linebuf, ",\n\0");
        while (ptoken) {
            this->f_lut[j] = (float) strtod (ptoken, NULL);
            ptoken = strtok (NULL, ",\n\0");
            j++;
        }
    }

    fclose (fp);
    return true;
}

bool
Rt_depth_dose::load_txt (const char* fn)
{
    char linebuf[128];
    FILE* fp = fopen (fn, "r");

    while (fgets (linebuf, 128, fp)) {
        float range, dose;
				float dose_int =0;

        if (2 != sscanf (linebuf, "%f %f", &range, &dose)) {
            break;
        }

				dose_int += dose;
        this->num_samples++;
        this->d_lut = (float*) realloc (
                        this->d_lut,
                        this->num_samples * sizeof(float));

        this->e_lut = (float*) realloc (
                        this->e_lut,
                        this->num_samples * sizeof(float));
				this->f_lut = (float*) realloc (
						this->f_lut,
						this->num_samples * sizeof(float));

        this->d_lut[this->num_samples-1] = range;
        this->e_lut[this->num_samples-1] = dose;
				this->f_lut[this->num_samples-1] = dose_int;
        this->dmax = range;         /* Assume entries are sorted */
    }

    fclose (fp);
    return true;
}

bool
Rt_depth_dose::generate ()
{
    int i;
    double d;

#if SPECFUN_FOUND
    if (!this->E0) {
        printf ("ERROR: Failed to generate beam -- energy not specified.\n");
        return false;
    }
    if (!this->spread) {
        printf ("ERROR: Failed to generate beam -- energy spread not specified.\n");
        return false;
    }
    if (!this->dmax) {
        printf ("ERROR: Failed to generate beam -- max depth not specified.\n");
        return false;
    }

    this->num_samples = (int) floorf (this->dmax / this->dres);

    this->d_lut = (float*) malloc (this->num_samples*sizeof(float));
    this->e_lut = (float*) malloc (this->num_samples*sizeof(float));
	this->f_lut = (float*) malloc (this->num_samples*sizeof(float));
    
    memset (this->d_lut, 0, this->num_samples*sizeof(float));
    memset (this->e_lut, 0, this->num_samples*sizeof(float));
	memset (this->f_lut, 0, this->num_samples*sizeof(float));

    for (d=0, i=0; i<this->num_samples; d+=this->dres, i++) {
        d_lut[i] = d;
		e_lut[i] = bragg_curve_norm (this->E0, this->spread, d)*this->dres;
		if (d == 0) {f_lut[i] = e_lut[i];} else {f_lut[i] = f_lut[i-1] + e_lut[i];}
    }
    return true;
#else
    printf ("ERROR: No specfun found.\n");
    return false;
#endif
}

void
Rt_depth_dose::dump (const char* fn) const
{
    FILE* fp = fopen (fn, "w");

    for (int i=0; i<this->num_samples; i++) {
       fprintf (fp, "%3.2f %3.2f\n", this->d_lut[i], this->e_lut[i]);
    }

    fclose (fp);
}

float
Rt_depth_dose::lookup_energy_integration (float depth, float dz) const
{	
    int i;
	int j;
    float energy = 0.0f;

	float dres = this->dres;

	float dmin = depth - dz/2.0;
	float dmax = depth + dz/2.0;

    /* Sanity check */
	if (depth + dz/2.0 < 0) {
		return 0.0f;
	}

    /* Find index into profile arrays */
    for (i = 0; i < this->num_samples-1; i++) {
        if (this->d_lut[i]> dmin) {
			i--;
            break;
        }
    }

	for (j = i; j < this->num_samples-1; j++) {
		if (this->d_lut[j] > dmax) {
			j--;
			break;
		}
	}

	/* Use index to lookup and interpolate energy */
    if (j >= 0 && j < this->num_samples-1) {
        // linear interpolation
        energy = this->f_lut[j]
            + (dmax - this->d_lut[j])
            * ((this->f_lut[j+1] - this->f_lut[j]) 
                / (this->d_lut[j+1] - this->d_lut[j]));
    } 
	else //(j == num_samples-1)
	{
		energy = this->f_lut[num_samples-1];
	}

	if (i >= 0 && i < this->num_samples-1) {
		// linear interpolation
        energy -= this->f_lut[i]
            + (dmin - this->d_lut[i])
            * ((this->f_lut[i+1] - this->f_lut[i]) 
                / (this->d_lut[i+1] - this->d_lut[i]));
    } 
	else if (i == num_samples-1) //(i == num_samples-1)
	{
		energy -= this->f_lut[num_samples-1];
	}

	return energy;  
}
