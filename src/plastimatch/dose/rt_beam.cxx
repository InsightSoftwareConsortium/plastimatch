/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bragg_curve.h"
#include "rt_beam.h"
#include "rt_plan.h"

class Rt_beam_private {
public:

    /* dose volume */
    Plm_image::Pointer dose_vol;

    double source[3];
    double isocenter[3];
    int detail;
    char flavor;
    char homo_approx;
    Particle_type part;

    float photon_energy; // energy for mono-energetic beams

    float beamWeight;

    Rt_sobp::Pointer sobp;

    std::string debug_dir;

    float smearing;

    float prescription_d_min;
    float prescription_d_max;
    float proximal_margin;
    float distal_margin;

    double step_length;
    float z_min;
    float z_max;
    float z_step;

		char rc_MC_model;
    float source_size;

    Aperture::Pointer aperture;
    Plm_image::Pointer target;

    std::string aperture_in;
    std::string range_compensator_in;

    std::string aperture_out;
    std::string proj_dose_out;
    std::string proj_img_out;
    std::string range_compensator_out;
    std::string sigma_out;
    std::string wed_out;

    /* When a new sobp is created from an existing sobp, 
       the peaks are copied (not manual).  Modifications of 
       an implicitly defined sobp (by adding a peak) will 
       delete any existing peaks. */
    bool have_copied_peaks;
	bool have_manual_peaks;
    bool have_prescription;

public:
    Rt_beam_private ()
    {
        this->dose_vol = Plm_image::New();

        this->source[0] = -1000.f;
        this->source[1] = 0.f;
        this->source[2] = 0.f;
        this->isocenter[0] = 0.f;
        this->isocenter[1] = 0.f;
        this->isocenter[2] = 0.f;
        this->detail = 1;
        this->flavor = 'a';
        this->homo_approx = 'n';
        this->part = PARTICLE_TYPE_P;

        this->photon_energy = 6.f; 

        this->beamWeight = 1.f;

        this->sobp = Rt_sobp::New();

        this->debug_dir = "";

        this->smearing = 0.f;

        this->prescription_d_min = 0.f;
        this->prescription_d_max = 0.f;
        this->proximal_margin = 0.f;
        this->distal_margin = 0.f;

        this->step_length = 1.0;
        this->z_min = 0.f;
        this->z_max = 100.f;
        this->z_step = 1.f;

				this->rc_MC_model = 'n';
        this->source_size = 0.f;

        aperture = Aperture::New();

        this->aperture_in = "";
        this->range_compensator_in = "";

        this->aperture_out = "";
        this->proj_dose_out = "";
        this->proj_img_out = "";
        this->range_compensator_out = "";
        this->sigma_out = "";
        this->wed_out = "";

        this->have_copied_peaks = false;
		this->have_manual_peaks = false;
        this->have_prescription = false;
    }
    Rt_beam_private (const Rt_beam_private* rtbp)
    {
        this->dose_vol = Plm_image::New();

        this->source[0] = rtbp->source[0];
        this->source[1] = rtbp->source[1];
        this->source[2] = rtbp->source[2];
        this->isocenter[0] = rtbp->isocenter[0];
        this->isocenter[1] = rtbp->isocenter[1];
        this->isocenter[2] = rtbp->isocenter[2];
        this->detail = rtbp->detail;
        this->flavor = rtbp->flavor;
        this->homo_approx = rtbp->homo_approx;
        this->part = rtbp->part;

        this->photon_energy = rtbp->photon_energy;

        this->beamWeight = rtbp->beamWeight;

        /* Copy the sobp object */
        this->sobp = Rt_sobp::New (rtbp->sobp);

        this->debug_dir = rtbp->debug_dir;

        this->smearing = rtbp->smearing;

        this->prescription_d_min = rtbp->prescription_d_min;
        this->prescription_d_max = rtbp->prescription_d_max;
        this->proximal_margin = rtbp->proximal_margin;
        this->distal_margin = rtbp->distal_margin;

        this->step_length = rtbp->step_length;
        this->z_min = rtbp->z_min;
        this->z_max = rtbp->z_max;
        this->z_step = rtbp->z_step;

        this->source_size = rtbp->source_size;

        /* Copy the aperture object */
        aperture = Aperture::New (rtbp->aperture);

        this->aperture_in = rtbp->aperture_in;
        this->range_compensator_in = rtbp->range_compensator_in;

        this->aperture_out = rtbp->aperture_out;
        this->proj_dose_out = rtbp->proj_dose_out;
        this->proj_img_out = rtbp->proj_img_out;
        this->range_compensator_out = rtbp->range_compensator_out;
        this->sigma_out = rtbp->sigma_out;
        this->wed_out = rtbp->wed_out;

        this->have_copied_peaks = true;
        this->have_prescription = rtbp->have_prescription;
    }
};

Rt_beam::Rt_beam ()
{
    this->d_ptr = new Rt_beam_private();
    this->rpl_vol = new Rpl_volume();

    /* Creation of the volumes useful for dose calculation */

    if (this->get_flavor() == 'a')
    {    
        this->aperture_vol = new Rpl_volume();
    }

    if (this->get_flavor() == 'f')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();
    }

    if (this->get_flavor() == 'g')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();

        this->rpl_vol_lg = new Rpl_volume();
        this->rpl_ct_vol_HU_lg = new Rpl_volume();
        this->sigma_vol_lg = new Rpl_volume();
        this->rpl_dose_vol = new Rpl_volume();
    }

    if (this->get_flavor() == 'h')
    {    
        this->rpl_ct_vol_HU = new Rpl_volume();
        this->sigma_vol = new Rpl_volume();

        this->rpl_vol_lg = new Rpl_volume();
        this->rpl_ct_vol_HU_lg = new Rpl_volume();
        this->sigma_vol_lg = new Rpl_volume();
        this->rpl_dose_vol = new Rpl_volume();

        this->aperture_vol = new Rpl_volume();
    }
}

Rt_beam::Rt_beam (const Rt_beam* rt_beam)
{
    /* Copy all the private settings (?) */
    this->d_ptr = new Rt_beam_private (rt_beam->d_ptr);
    
    /* The below calculation volumes don't need to be copied 
       from input beam */
    this->rpl_vol = 0;
    this->rpl_ct_vol_HU = 0;
    this->sigma_vol = 0;
    this->rpl_vol_lg = 0;
    this->rpl_ct_vol_HU_lg = 0;
    this->sigma_vol_lg = 0;
    this->rpl_dose_vol = 0;
    this->aperture_vol = 0;
}

Rt_beam::~Rt_beam ()
{
    delete this->d_ptr;
}

bool
Rt_beam::load (const char* fn)
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

const double*
Rt_beam::get_source_position () const
{
    return d_ptr->source;
}

double
Rt_beam::get_source_position (int dim) const
{
    return d_ptr->source[dim];
}

void
Rt_beam::set_source_position (const float* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->source[d] = position[d];
    }
}

void
Rt_beam::set_source_position (const double* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->source[d] = position[d];
    }
}

const double*
Rt_beam::get_isocenter_position () const
{
    return d_ptr->isocenter;
}

double
Rt_beam::get_isocenter_position (int dim) const
{
    return d_ptr->isocenter[dim];
}

void
Rt_beam::set_isocenter_position (const float* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->isocenter[d] = position[d];
    }
}

void
Rt_beam::set_isocenter_position (const double* position)
{
    for (int d = 0; d < 3; d++) {
        d_ptr->isocenter[d] = position[d];
    }
}

void
Rt_beam::add_peak (
    double E0,                      /* initial ion energy (MeV) */
    double spread,                  /* beam energy sigma (MeV) */
    double dres,                    /* spatial resolution of bragg curve (mm)*/
    double dmax,                    /* maximum w.e.d. (mm) */
    double weight)
{
    if (d_ptr->have_copied_peaks == true) {
        d_ptr->sobp->clear_peaks ();
    }
    d_ptr->have_copied_peaks = false;

    d_ptr->sobp->add_peak (E0, spread, dres, dmax, weight);
}

int
Rt_beam::get_detail (void) const
{
    return d_ptr->detail;
}

void
Rt_beam::set_detail (int detail)
{
    d_ptr->detail = detail;
}

char
Rt_beam::get_flavor (void) const
{
    return d_ptr->flavor;
}

void
Rt_beam::set_flavor (char flavor)
{
    d_ptr->flavor = flavor;
}

char 
Rt_beam::get_homo_approx () const
{
    return d_ptr->homo_approx;
}
    
void 
Rt_beam::set_homo_approx (char homo_approx)
{
  d_ptr->homo_approx = homo_approx;
}

double 
Rt_beam::get_sobp_maximum_depth ()
{
    return d_ptr->sobp->get_maximum_depth ();
}

Rt_sobp::Pointer
Rt_beam::get_sobp()
{
	return d_ptr->sobp;
}

float
Rt_beam::get_beam_weight (void) const
{
    return d_ptr->beamWeight;
}

void
Rt_beam::set_beam_weight (float beamWeight)
{
    d_ptr->beamWeight = beamWeight;
}

float 
Rt_beam::get_proximal_margin()
{
    return d_ptr->proximal_margin;
}
	
float 
Rt_beam::get_distal_margin()
{
    return d_ptr->distal_margin;
}

float 
Rt_beam::get_prescription_min()
{
    return d_ptr->prescription_d_min;
}

void
Rt_beam::set_prescription_min (float prescription_min)
{
    d_ptr->prescription_d_min = prescription_min;
    d_ptr->have_prescription = true;
}

float 
Rt_beam::get_prescription_max()
{
    return d_ptr->prescription_d_max;
}

void
Rt_beam::set_prescription_max (float prescription_max)
{
    d_ptr->prescription_d_max = prescription_max;
    d_ptr->have_prescription = true;
}

void
Rt_beam::set_proximal_margin (float proximal_margin)
{
    d_ptr->proximal_margin = proximal_margin;
    d_ptr->sobp->set_prescription_min_max (
        d_ptr->prescription_d_min - d_ptr->proximal_margin,
        d_ptr->prescription_d_max + d_ptr->distal_margin);
}

void
Rt_beam::set_distal_margin (float distal_margin)
{
    d_ptr->distal_margin = distal_margin;
    d_ptr->sobp->set_prescription_min_max (
        d_ptr->prescription_d_min - d_ptr->proximal_margin,
        d_ptr->prescription_d_max + d_ptr->distal_margin);
}

void 
Rt_beam::set_sobp_prescription_min_max (float d_min, float d_max)
{
    d_ptr->prescription_d_min = d_min;
    d_ptr->prescription_d_max = d_max;
    d_ptr->sobp->set_prescription_min_max (
        d_ptr->prescription_d_min - d_ptr->proximal_margin,
        d_ptr->prescription_d_max + d_ptr->distal_margin);
}

void
Rt_beam::set_rc_MC_model (char rc_MC_model)
{
    d_ptr->rc_MC_model = rc_MC_model;
}

char
Rt_beam::get_rc_MC_model (void) const
{
    return d_ptr->rc_MC_model;
}

void
Rt_beam::set_source_size(float source_size)
{
    d_ptr->source_size = source_size;
}

float
Rt_beam::get_source_size() const
{
    return d_ptr->source_size;
}

void
Rt_beam::set_debug (const std::string& dir)
{
    d_ptr->debug_dir = dir;
}

void
Rt_beam::optimize_sobp ()
{
    d_ptr->sobp->optimize ();
}

bool
Rt_beam::generate ()
{
    return d_ptr->sobp->generate ();
}

void
Rt_beam::dump (const char* dir)
{
    d_ptr->sobp->dump (dir);
}

float
Rt_beam::lookup_sobp_dose (
    float depth
)
{
    return d_ptr->sobp->lookup_energy(depth);
}

void
Rt_beam::compute_beam_modifiers ()
{
    /* Compute the aperture and compensator */
    this->rpl_vol->compute_beam_modifiers (
        this->get_target()->get_vol(), 0);
    /* Apply smearing */
    d_ptr->aperture->apply_smearing (d_ptr->smearing);
}

void
Rt_beam::apply_beam_modifiers ()
{
    this->rpl_vol->apply_beam_modifiers ();
}

Plm_image::Pointer&
Rt_beam::get_target ()
{
    return d_ptr->target;
}

const Plm_image::Pointer&
Rt_beam::get_target () const 
{
    return d_ptr->target;
}

void 
Rt_beam::set_target(Plm_image::Pointer& target)
{
    d_ptr->target = target;
}

Plm_image::Pointer&
Rt_beam::get_dose ()
{
    return d_ptr->dose_vol;
}

const Plm_image::Pointer&
Rt_beam::get_dose () const 
{
    return d_ptr->dose_vol;
}

void 
Rt_beam::set_dose(Plm_image::Pointer& dose)
{
    d_ptr->dose_vol = dose;
}

Aperture::Pointer&
Rt_beam::get_aperture () 
{
    return d_ptr->aperture;
}

const Aperture::Pointer&
Rt_beam::get_aperture () const
{
    return d_ptr->aperture;
}

void
Rt_beam::set_aperture_vup (const float vup[])
{
    d_ptr->aperture->set_vup (vup);
}

void
Rt_beam::set_aperture_distance (float ap_distance)
{
    d_ptr->aperture->set_distance (ap_distance);
}

void
Rt_beam::set_aperture_origin (const float ap_origin[])
{
    this->get_aperture()->set_origin (ap_origin);
}

void
Rt_beam::set_aperture_resolution (const int ap_resolution[])
{
    this->get_aperture()->set_dim (ap_resolution);
}

void
Rt_beam::set_aperture_spacing (const float ap_spacing[])
{
    this->get_aperture()->set_spacing (ap_spacing);
}

void
Rt_beam::set_smearing (float smearing)
{
    d_ptr->smearing = smearing;
}

float 
Rt_beam::get_smearing()
{
    return d_ptr->smearing;
}

void
Rt_beam::set_step_length (double step_length)
{
    d_ptr->step_length = step_length;
}

double 
Rt_beam::get_step_length()
{
    return d_ptr->step_length;
}

void
Rt_beam::set_beam_depth (float z_min, float z_max, float z_step)
{
    d_ptr->z_min = z_min;
    d_ptr->z_max = z_max;
    d_ptr->z_step = z_step;
}

void 
Rt_beam::set_particle_type(Particle_type particle_type)
{
    d_ptr->part = particle_type;
}

Particle_type 
Rt_beam::get_particle_type()
{
    return d_ptr->part;
}

void 
Rt_beam::set_aperture_in (const std::string& str)
{
    d_ptr->aperture_in = str;
}

std::string 
Rt_beam::get_aperture_in()
{
    return d_ptr->aperture_in;
}

void 
Rt_beam::set_range_compensator_in (const std::string& str)
{
    d_ptr->range_compensator_in = str;
}

std::string 
Rt_beam::get_range_compensator_in()
{
    return d_ptr->range_compensator_in;
}

void 
Rt_beam::set_aperture_out(std::string str)
{
    d_ptr->aperture_out = str;
}

std::string 
Rt_beam::get_aperture_out()
{
    return d_ptr->aperture_out;
}

void 
Rt_beam::set_proj_dose_out(std::string str)
{
    d_ptr->proj_dose_out = str;
}

std::string 
Rt_beam::get_proj_dose_out()
{
    return d_ptr->proj_dose_out;
}

void 
Rt_beam::set_proj_img_out(std::string str)
{
    d_ptr->proj_img_out = str;
}

std::string 
Rt_beam::get_proj_img_out()
{
    return d_ptr->proj_img_out;
}

void 
Rt_beam::set_range_compensator_out(std::string str)
{
    d_ptr->range_compensator_out = str;
}

std::string 
Rt_beam::get_range_compensator_out()
{
    return d_ptr->range_compensator_out;
}

void 
Rt_beam::set_sigma_out(std::string str)
{
    d_ptr->sigma_out = str;
}

std::string 
Rt_beam::get_sigma_out()
{
    return d_ptr->sigma_out;
}

void 
Rt_beam::set_wed_out(std::string str)
{
    d_ptr->wed_out = str;
}

std::string 
Rt_beam::get_wed_out()
{
    return d_ptr->wed_out;
}

void 
Rt_beam::set_photon_energy(float energy)
{
    d_ptr->photon_energy = energy;
}

float 
Rt_beam::get_photon_energy()
{
    return d_ptr->photon_energy;
}

void 
Rt_beam::set_have_prescription(bool have_prescription)
{
    d_ptr->have_prescription = have_prescription;
}

bool 
Rt_beam::get_have_prescription()
{
    return d_ptr->have_prescription;
}

void 
Rt_beam::set_have_copied_peaks(bool have_copied_peaks)
{
    d_ptr->have_copied_peaks = have_copied_peaks;
}
	
bool 
Rt_beam::get_have_copied_peaks()
{
    return d_ptr->have_copied_peaks;
}

void 
Rt_beam::set_have_manual_peaks(bool have_manual_peaks)
{
    d_ptr->have_manual_peaks = have_manual_peaks;
}

bool 
Rt_beam::get_have_manual_peaks()
{
    return d_ptr->have_manual_peaks;
}

void 
Rt_beam::copy_sobp(Rt_sobp::Pointer sobp)
{
    d_ptr->sobp->set_dose_lut(sobp->get_d_lut(), sobp->get_e_lut(), sobp->get_num_samples()); /* copy also num_samples */
    d_ptr->sobp->set_dres(sobp->get_dres());
    d_ptr->sobp->set_eres(sobp->get_eres());
    d_ptr->sobp->set_E_min(sobp->get_E_min());
    d_ptr->sobp->set_E_max(sobp->get_E_max());
    d_ptr->sobp->set_dmin(sobp->get_dmin());
    d_ptr->sobp->set_dmax(sobp->get_dmax());
    d_ptr->sobp->set_dend(sobp->get_dend());
    d_ptr->sobp->set_particle_type(sobp->get_particle_type());
    d_ptr->sobp->set_p(sobp->get_p());
    d_ptr->sobp->set_alpha(sobp->get_alpha());
    d_ptr->sobp->set_prescription_min(sobp->get_prescription_min());
    d_ptr->sobp->set_prescription_max(sobp->get_prescription_max());
    d_ptr->sobp->add_weight(sobp->get_weight()[sobp->get_num_peaks()-1]);
    d_ptr->sobp->add_depth_dose(sobp->get_depth_dose()[sobp->get_num_peaks()-1]);	
}

bool
Rt_beam::load_xio (const char* fn)
{
#if defined (commentout)
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
    
    memset (this->d_lut, 0, this->num_samples*sizeof(float));
    memset (this->e_lut, 0, this->num_samples*sizeof(float));

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

    fclose (fp);
#endif
    return true;
}

bool
Rt_beam::load_txt (const char* fn)
{
#if defined (commentout)
    char linebuf[128];
    FILE* fp = fopen (fn, "r");

    while (fgets (linebuf, 128, fp)) {
        float range, dose;

        if (2 != sscanf (linebuf, "%f %f", &range, &dose)) {
            break;
        }

        this->num_samples++;
        this->d_lut = (float*) realloc (
                        this->d_lut,
                        this->num_samples * sizeof(float));

        this->e_lut = (float*) realloc (
                        this->e_lut,
                        this->num_samples * sizeof(float));

        this->d_lut[this->num_samples-1] = range;
        this->e_lut[this->num_samples-1] = dose;
        this->dmax = range;         /* Assume entries are sorted */
    }

    fclose (fp);
#endif
    return true;
}

