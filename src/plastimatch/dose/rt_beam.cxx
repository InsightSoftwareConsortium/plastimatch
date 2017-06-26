/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string_util.h>
#include <math.h>

#include "bragg_curve.h"
#include "plm_math.h"
#include "proj_volume.h"
#include "rt_beam.h"
#include "rt_dose_timing.h"
#include "rt_plan.h"
#include "rt_spot_map.h"

static void
save_vector_as_image (
    const std::vector<double>& v,
    const int dim2d[2],
    const std::string& filename)
{
    plm_long dim[3] = { dim2d[0], dim2d[1], 1 };
    float origin[3] = { 0.f, 0.f, 0.f };
    float spacing[3] = { 1.f, 1.f, 1.f };
    Volume::Pointer vol = Volume::New (
        dim, origin, spacing, (float*) 0, PT_FLOAT, 1);
    float *vol_img = vol->get_raw<float> ();

    for (plm_long i = 0; i < vol->npix; i++)
    {
        if (std::isnan(v[i]) || std::isinf(v[i]) || v[i] == NLMAX(double)) {
            vol_img[i] = -1;
        } else {
            vol_img[i] = (float) v[i];
        }
    }

    Plm_image::Pointer img = Plm_image::New (vol);
    img->save_image (filename);
}


class Rt_beam_private {
public:

    /* dose volume */
    Plm_image::Pointer dose_vol;

    double source[3];
    double isocenter[3];
    char flavor;
    char homo_approx;

    float beamWeight;

    Rt_spot_map::Pointer spot_map;
    Rt_mebs::Pointer mebs;
    std::string debug_dir;

    float smearing;
    char rc_MC_model;
    float source_size;
    float step_length;

    double max_wed;
    double min_wed;

    Plm_image::Pointer ct_hu;
    Plm_image::Pointer ct_psp;
    Plm_image::Pointer target;
    Aperture::Pointer aperture;

    Rt_dose_timing::Pointer rt_dose_timing;

    std::string aperture_in;
    std::string range_compensator_in;

    std::string aperture_out;
    std::string proj_dose_out;
    std::string proj_img_out;
    std::string proj_target_out;
    std::string range_compensator_out;
    std::string sigma_out;
    std::string wed_out;
    std::string beam_dump_out;
    
    std::string beam_line_type;

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
        this->flavor = 'a';
        this->homo_approx = 'n';

        this->beamWeight = 1.f;
        this->mebs = Rt_mebs::New();
        this->debug_dir = "";
        this->smearing = 0.f;
        this->rc_MC_model = 'n';
        this->source_size = 0.f;
        this->step_length = 1.f;
        this->min_wed = 0.;
        this->max_wed = 0.;

        aperture = Aperture::New();
        rt_dose_timing = Rt_dose_timing::New();

        this->aperture_in = "";
        this->range_compensator_in = "";
        this->aperture_out = "";
        this->proj_dose_out = "";
        this->proj_img_out = "";
        this->proj_target_out = "";
        this->range_compensator_out = "";
        this->sigma_out = "";
        this->wed_out = "";
        this->beam_line_type = "active";
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
        this->flavor = rtbp->flavor;
        this->homo_approx = rtbp->homo_approx;

        /* Copy the mebs object */
        this->beamWeight = rtbp->beamWeight;
        this->mebs = Rt_mebs::New (rtbp->mebs);
        this->debug_dir = rtbp->debug_dir;
        this->smearing = rtbp->smearing;
        this->source_size = rtbp->source_size;
        this->step_length = rtbp->step_length;
        this->min_wed = rtbp->min_wed;
        this->max_wed = rtbp->max_wed;

        /* Copy the aperture object */
        aperture = Aperture::New (rtbp->aperture);

        /* Share same timing object */
        rt_dose_timing = rtbp->rt_dose_timing;

        this->aperture_in = rtbp->aperture_in;
        this->range_compensator_in = rtbp->range_compensator_in;
        this->aperture_out = rtbp->aperture_out;
        this->proj_dose_out = rtbp->proj_dose_out;
        this->proj_img_out = rtbp->proj_img_out;
        this->proj_target_out = rtbp->proj_target_out;
        this->range_compensator_out = rtbp->range_compensator_out;
        this->sigma_out = rtbp->sigma_out;
        this->wed_out = rtbp->wed_out;
        this->beam_line_type = rtbp->beam_line_type;
    }
};

Rt_beam::Rt_beam ()
{
    this->d_ptr = new Rt_beam_private();

    /* Creation of the volumes useful for dose calculation */
    this->rsp_accum_vol = new Rpl_volume();
    this->hu_samp_vol = 0;
    this->sigma_vol = 0;
    this->rpl_vol_lg = 0;
    this->rpl_vol_samp_lg = 0;
    this->sigma_vol_lg = 0;
    this->rpl_dose_vol = 0;
}

Rt_beam::Rt_beam (const Rt_beam* rt_beam)
{
    /* Copy all the private settings (?) */
    this->d_ptr = new Rt_beam_private (rt_beam->d_ptr);
    
    /* The below calculation volumes don't need to be copied 
       from input beam */
    this->rsp_accum_vol = 0;
    this->hu_samp_vol = 0;
    this->sigma_vol = 0;
    this->rpl_vol_lg = 0;
    this->rpl_vol_samp_lg = 0;
    this->sigma_vol_lg = 0;
    this->rpl_dose_vol = 0;
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

double 
Rt_beam::get_source_distance () const
{
    return vec3_dist (d_ptr->isocenter, d_ptr->source);
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

Rt_mebs::Pointer
Rt_beam::get_mebs()
{
    return d_ptr->mebs;
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
Rt_beam::dump (const char* dir)
{
    d_ptr->mebs->dump (dir);
}

void
Rt_beam::dump (const std::string& dir)
{
    this->dump (dir.c_str());
}

bool
Rt_beam::prepare_for_calc (
    Plm_image::Pointer& ct_hu,
    Plm_image::Pointer& ct_psp,
    Plm_image::Pointer& target)
{
    if (!ct_hu) return false;
    if (!ct_psp) return false;
    d_ptr->ct_hu = ct_hu;
    d_ptr->ct_psp = ct_psp;
    d_ptr->target = target;

    if (this->get_aperture()->get_distance() > this->get_source_distance ()) {
        lprintf ("Source distance must be greater than aperture distance");
        return false;
    }

    Rpl_volume_ray_trace_start rvrts = RAY_TRACE_START_AT_CLIPPING_PLANE;
    
    // Create rsp_accum_vol */
    if (!this->rsp_accum_vol) {
        this->rsp_accum_vol = new Rpl_volume;
    }
    if (!this->rsp_accum_vol) return false;
    this->rsp_accum_vol->set_geometry (
        this->get_source_position(),
        this->get_isocenter_position(),
        this->get_aperture()->vup,
        this->get_aperture()->get_distance(),
        this->get_aperture()->get_dim(),
        this->get_aperture()->get_center(),
        this->get_aperture()->get_spacing(),
        this->get_step_length());
    this->rsp_accum_vol->set_aperture (this->get_aperture());
    this->rsp_accum_vol->set_ray_trace_start (rvrts);
    this->rsp_accum_vol->set_ct_volume (ct_psp);
    if (!this->rsp_accum_vol->get_ct() || !this->rsp_accum_vol->get_ct_limit()) {
        lprintf ("ray_data or clipping planes missing from rpl volume\n");
        return false;
    }
    this->rsp_accum_vol->compute_rpl_accum (false);

    // Create ct projective volume
    // GCS FIX: The old code re-used the ray data.  Is that really faster?
    this->hu_samp_vol = new Rpl_volume;
    if (!this->hu_samp_vol) return false;
    this->hu_samp_vol->clone_geometry (this->rsp_accum_vol);
    this->hu_samp_vol->set_ray_trace_start (rvrts);
    this->hu_samp_vol->set_aperture (this->get_aperture());
    this->hu_samp_vol->set_ct_volume (d_ptr->ct_hu);
    this->hu_samp_vol->compute_rpl_sample (false);

    // Prepare, but don't compute the sigma volume yet
    if (this->get_flavor() == 'd'
        || this->get_flavor() == 'f'
        || this->get_flavor() == 'g'
        || this->get_flavor() == 'h')
    {
        this->sigma_vol = new Rpl_volume;
        if (!this->sigma_vol) return false;
        this->sigma_vol->clone_geometry (this->rsp_accum_vol);
        this->sigma_vol->set_aperture (this->get_aperture());
        this->sigma_vol->set_ct_volume (d_ptr->ct_hu);
        this->sigma_vol->set_ct_limit(this->rsp_accum_vol->get_ct_limit());
        this->sigma_vol->compute_ray_data();
        this->sigma_vol->set_front_clipping_plane(this->rsp_accum_vol->get_front_clipping_plane());
        this->sigma_vol->set_back_clipping_plane(this->rsp_accum_vol->get_back_clipping_plane());
        this->sigma_vol->compute_rpl_void();
    }

    // Create target projective volume */
    if (d_ptr->target) {
        this->target_rv = Rpl_volume::New();
        if (!this->target_rv) return false;
        this->target_rv->clone_geometry (this->rsp_accum_vol);
        this->target_rv->set_ray_trace_start (rvrts);
        this->target_rv->set_aperture (this->get_aperture());
        this->target_rv->set_ct_volume (d_ptr->target);
        this->target_rv->compute_rpl_sample (false);
    }

    // Create and fill in rpl_dose_volume (actually proj dose)
    if (this->get_flavor() == 'b'
        || this->get_flavor() == 'd')
    {
        this->rpl_dose_vol = new Rpl_volume;
        if (!this->rpl_dose_vol) return false;
        this->rpl_dose_vol->clone_geometry (this->rsp_accum_vol);
        this->rpl_dose_vol->set_ray_trace_start (rvrts);
        this->rpl_dose_vol->set_aperture (this->get_aperture());
        this->rpl_dose_vol->set_ct_volume (d_ptr->ct_hu);
        this->rpl_dose_vol->set_ct_limit(this->rsp_accum_vol->get_ct_limit());
        this->rpl_dose_vol->compute_ray_data();
        this->rpl_dose_vol->set_front_clipping_plane(this->rsp_accum_vol->get_front_clipping_plane());
        this->rpl_dose_vol->set_back_clipping_plane(this->rsp_accum_vol->get_back_clipping_plane());
        this->rpl_dose_vol->compute_rpl_void();
    }

    /* Next, depending on what the user asked for, we may create apertures, 
       range compensators, use pre-defined apertures or spot maps, etc. */
    if (d_ptr->mebs->get_have_particle_number_map() == true
        && d_ptr->beam_line_type == "passive")
    {
        printf("***WARNING*** Passively scattered beam line with spot map file detected: %s.\nBeam line set to active scanning.\n", d_ptr->mebs->get_particle_number_in().c_str());
        printf("Any manual peaks set, depth prescription, target or range compensator will not be considered.\n");
        this->compute_beam_data_from_spot_map();
        return true;
    }

   /* The priority how to generate dose is:
       1. manual spot map 
       2. manual peaks 
       3. dose prescription 
       4. target 
       5. 100 MeV sample beam */
    if (d_ptr->mebs->get_have_particle_number_map() == true)
    {
        printf("Spot map file detected: Any manual peaks set, depth prescription, target or range compensator will not be considered.\n");
        this->compute_beam_data_from_spot_map();
        return true;
    }
    if (d_ptr->mebs->get_have_manual_peaks() == true)
    {
        printf("Manual peaks detected [PEAKS]: Any prescription or target depth will not be considered.\n");
        this->get_mebs()->set_have_manual_peaks(true);
        this->compute_beam_data_from_manual_peaks(target);
        return true;
    }
    if (d_ptr->mebs->get_have_prescription() == true)
    {
        printf("Prescription depths detected. Any target depth will not be considered.\n");
        this->get_mebs()->set_have_prescription(true);
        /* Apply margins */
        this->get_mebs()->set_target_depths(d_ptr->mebs->get_prescription_min(), d_ptr->mebs->get_prescription_max());
        this->compute_beam_data_from_prescription(target);
        return true;
    }
    if (target->get_vol())
    {
        printf("Target detected.\n");
        this->get_mebs()->set_have_manual_peaks(false);
        this->get_mebs()->set_have_prescription(false);
        this->compute_beam_data_from_target(target);
        return true;
    }

    /* If we arrive to this point, it is because no beam was defined
       Creation of a default beam: 100 MeV */
    printf("***WARNING*** No spot map, manual peaks, depth prescription or target detected.\n");
    printf("Beam set to a 100 MeV mono-energetic beam. Proximal and distal margins not considered.\n");
    this->compute_default_beam();

    return true;
}

void
Rt_beam::compute_beam_data_from_spot_map()
{
    this->get_mebs()->clear_depth_dose();
    this->get_mebs()->extract_particle_number_map_from_txt(this->get_aperture());

    /* the automatic aperture and range compensator are erased and the 
       ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_manual_peaks(Plm_image::Pointer& target)
{
    /* The spot map will be identical for passive or scanning beam lines */
    int ap_dim[2] = {this->get_aperture()->get_dim()[0], this->get_aperture()->get_dim()[1]};
    this->get_mebs()->generate_part_num_from_weight(ap_dim);
    if ((target->get_vol() && (d_ptr->aperture_in =="" || d_ptr->range_compensator_in =="")) && (d_ptr->mebs->get_have_manual_peaks() == true || d_ptr->mebs->get_have_prescription() == true)) // we build the associate range compensator and aperture
    {
        if (d_ptr->beam_line_type == "active")
        {
            this->compute_beam_modifiers_active_scanning (
                target->get_vol(), d_ptr->smearing, 
                d_ptr->mebs->get_proximal_margin(), 
                d_ptr->mebs->get_distal_margin());
        }
        else
        {
            this->compute_beam_modifiers_passive_scattering (
                target->get_vol(), d_ptr->smearing, 
                d_ptr->mebs->get_proximal_margin(), 
                d_ptr->mebs->get_distal_margin());
        }
    }
    /* the automatic aperture and range compensator are erased and the 
       ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_manual_peaks()
{
    /* The spot map will be identical for passive or scanning beam lines */
    int ap_dim[2] = {this->get_aperture()->get_dim()[0], this->get_aperture()->get_dim()[1]};
    this->get_mebs()->generate_part_num_from_weight(ap_dim);
    /* the automatic aperture and range compensator are erased and the 
       ones defined in the input file are considered */
    this->update_aperture_and_range_compensator();
}

void
Rt_beam::compute_beam_data_from_prescription(Plm_image::Pointer& target)
{
    /* The spot map will be identical for passive or scanning beam lines */
    /* Identic to compute from manual peaks, with a preliminary optimization */
    d_ptr->mebs->optimize_sobp();
    this->compute_beam_data_from_manual_peaks(target);
}

void
Rt_beam::compute_beam_data_from_target(Plm_image::Pointer& target)
{
    /* Compute beam aperture, range compensator 
       + SOBP for passively scattered beam lines */
    if (this->get_beam_line_type() == "passive")
    {
        this->compute_beam_modifiers (
            d_ptr->target->get_vol(), this->get_mebs()->get_min_wed_map(),
            this->get_mebs()->get_max_wed_map());
        this->compute_beam_data_from_prescription (target);
    }
    else
    {
        std::vector <double> wepl_min;
        std::vector <double> wepl_max;
        this->compute_beam_modifiers_active_scanning (
            target->get_vol(), d_ptr->smearing,
            d_ptr->mebs->get_proximal_margin(),
            d_ptr->mebs->get_distal_margin(),
            wepl_min, wepl_max);

        d_ptr->mebs->compute_particle_number_matrix_from_target_active (
            this->rsp_accum_vol, wepl_min, wepl_max);
    }
}

void 
Rt_beam::compute_default_beam()
{
    /* Computes a default 100 MeV peak */
    this->get_mebs()->add_peak(100, 1, 1);
    this->compute_beam_data_from_manual_peaks();
}

void 
Rt_beam::compute_beam_modifiers (Volume *seg_vol)
{
    if (d_ptr->beam_line_type == "active")
    {
        this->compute_beam_modifiers_active_scanning (seg_vol, 
            d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), 
            d_ptr->mebs->get_distal_margin());
    }
    else
    {
        this->compute_beam_modifiers_passive_scattering (seg_vol, 
            d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), 
            d_ptr->mebs->get_distal_margin());
    }

    d_ptr->mebs->set_prescription_depths (d_ptr->min_wed, d_ptr->max_wed);
    this->rsp_accum_vol->apply_beam_modifiers ();
}

void 
Rt_beam::compute_beam_modifiers (Volume *seg_vol, std::vector<double>& map_wed_min, std::vector<double>& map_wed_max)
{
    if (d_ptr->beam_line_type == "active")
    {
        this->compute_beam_modifiers_active_scanning (
            seg_vol, d_ptr->smearing,
            d_ptr->mebs->get_proximal_margin(),
            d_ptr->mebs->get_distal_margin(), map_wed_min, map_wed_max);
    }
    else
    {
        this->compute_beam_modifiers_passive_scattering (seg_vol, 
            d_ptr->smearing, d_ptr->mebs->get_proximal_margin(), 
            d_ptr->mebs->get_distal_margin(), map_wed_min, map_wed_max);
    }
    d_ptr->mebs->set_prescription_depths (d_ptr->min_wed, d_ptr->max_wed);
    this->rsp_accum_vol->apply_beam_modifiers ();
}

void 
Rt_beam::compute_beam_modifiers_active_scanning (
    Volume *seg_vol, float smearing, float proximal_margin,
    float distal_margin)
{
    std::vector<double> map_wed_min;
    std::vector<double> map_wed_max;
    this->compute_beam_modifiers_core (seg_vol, true, smearing, proximal_margin,
        distal_margin, map_wed_min, map_wed_max);
}

void 
Rt_beam::compute_beam_modifiers_passive_scattering (
    Volume *seg_vol, float smearing, float proximal_margin, 
    float distal_margin)
{
    std::vector<double> map_wed_min;
    std::vector<double> map_wed_max;
    this-> compute_beam_modifiers_core (seg_vol, false, smearing, 
        proximal_margin, distal_margin, map_wed_min, map_wed_max);
}

void 
Rt_beam::compute_beam_modifiers_active_scanning (
    Volume *seg_vol, float smearing, float proximal_margin,
    float distal_margin, std::vector<double>& map_wed_min,
    std::vector<double>& map_wed_max)
{
    this->compute_beam_modifiers_core (seg_vol, true, smearing, proximal_margin,
        distal_margin, map_wed_min, map_wed_max);
}

void 
Rt_beam::compute_beam_modifiers_passive_scattering (
    Volume *seg_vol, float smearing, float proximal_margin, 
    float distal_margin, std::vector<double>& map_wed_min, 
    std::vector<double>& map_wed_max)
{
    this-> compute_beam_modifiers_core (seg_vol, false, smearing, 
        proximal_margin, distal_margin, map_wed_min, map_wed_max);
}

void
Rt_beam::compute_beam_modifiers_core (
    Volume *seg_vol,
    bool active,
    float smearing,
    float proximal_margin,
    float distal_margin,
    std::vector<double>& map_wed_min,
    std::vector<double>& map_wed_max)
{

    /* compute the target min and max distance (not wed!) map in the aperture */
    // GCS FIX: definitely we want wed here.
    // compute_target_distance_limits (seg_vol, map_wed_min, map_wed_max);
    printf("Compute target wepl_min_max...\n");
    this->compute_target_wepl_min_max (map_wed_min, map_wed_max);

#if defined (commentout)
    save_vector_as_image (
        map_wed_min,
        d_ptr->aperture->get_dim(),
        "debug-min-a.nrrd");
    save_vector_as_image (
        map_wed_max,
        d_ptr->aperture->get_dim(),
        "debug-max-a.nrrd");
#endif
    
    printf ("Apply lateral smearing to the target...\n");
    /* widen the min/max distance maps */
    if (smearing > 0) {
        this->apply_smearing_to_target (smearing, map_wed_min, map_wed_max);
    }

#if defined (commentout)
    save_vector_as_image (
        map_wed_min,
        d_ptr->aperture->get_dim(),
        "debug-min-b.nrrd");
    save_vector_as_image (
        map_wed_max,
        d_ptr->aperture->get_dim(),
        "debug-max-b.nrrd");
#endif
    
    printf ("Apply proximal and distal ...\n");
    /* add the margins */
    for (size_t i = 0; i < map_wed_min.size(); i++) {
        map_wed_min[i] -= proximal_margin;
        if (map_wed_min[i] < 0) {
            map_wed_min[i] = 0;
        }
        if (map_wed_max[i] > 0) {
            map_wed_max[i] += distal_margin;
        }
    }

#if defined (commentout)
    save_vector_as_image (
        map_wed_min,
        d_ptr->aperture->get_dim(),
        "debug-min-c.nrrd");
    save_vector_as_image (
        map_wed_max,
        d_ptr->aperture->get_dim(),
        "debug-max-c.nrrd");
#endif
    
    /* Compute max wed, used by range compensator */
    int idx = 0;
    double max_wed = 0;
    int i[2] = {0, 0};
    printf("Compute max wed...\n");
    for (i[1] = 0; i[1] < d_ptr->aperture->get_aperture_volume()->dim[1]; i[1]++) {
        for (i[0] = 0; i[0] < d_ptr->aperture->get_aperture_volume()->dim[0]; i[0]++) {
            idx = i[0] + i[1] * d_ptr->aperture->get_aperture_volume()->dim[0];
            if (map_wed_max[idx] > max_wed) {
                max_wed = map_wed_max[idx];
            }
        }
    }

    printf("Compute the aperture...\n");
    /* compute the aperture */
    /* This assumes that dim & spacing are correctly set in aperture */
    d_ptr->aperture->allocate_aperture_images ();

    Volume::Pointer aperture_vol = d_ptr->aperture->get_aperture_volume ();
    unsigned char *aperture_img = (unsigned char*) aperture_vol->img;
    for (int i = 0; i < aperture_vol->dim[0] * aperture_vol->dim[1]; i++) {
        if (map_wed_min[i] > 0) {
            aperture_img[i] = 1;
        }
        else {
            aperture_img[i] = 0;
        }
    }

    /* compute the range compensator if passive beam line with PMMA 
       range compensator */
    Volume::Pointer range_comp_vol = d_ptr->aperture->get_range_compensator_volume ();
    float *range_comp_img = (float*) range_comp_vol->img;
	
    if (active == false)
    {
        printf("Compute range compensator...\n");
    }

    for (int i = 0; i < aperture_vol->dim[0] * aperture_vol->dim[1]; i++)
    {
        if (active == true)
        {
            range_comp_img[i] = 0;
        }
        else 
        {
            range_comp_img[i] = (max_wed - map_wed_max[i]) / (PMMA_STPR * PMMA_DENSITY);
        }
    }

    /* compute the max/min wed of the entire target + margins + range_comp*/
    double total_min_wed = 0;
    double total_max_wed = 0;
    // Max should be the same as the max in the target as for this ray rgcomp is null
    for (int i = 0; i < aperture_vol->dim[0] * aperture_vol->dim[1]; i++)
    {
        if (range_comp_img[i] * PMMA_STPR * PMMA_DENSITY + map_wed_max[i] > total_max_wed) { // if active beam line, range comp is null
            total_max_wed = range_comp_img[i] * PMMA_STPR * PMMA_DENSITY + map_wed_max[i];
        }
    }
    total_min_wed = total_max_wed;
    for (int i = 0; i < aperture_vol->dim[0] * aperture_vol->dim[1]; i++)
    {
        if ((range_comp_img[i] * PMMA_STPR * PMMA_DENSITY + map_wed_max[i] > 0) && (range_comp_img[i] * PMMA_STPR * PMMA_DENSITY + map_wed_min[i] < total_min_wed)) {
            total_min_wed = range_comp_img[i] * PMMA_STPR * PMMA_DENSITY + map_wed_min[i];
        }
    }

    printf("Max wed in the target is %lg mm.\n", total_max_wed);
    printf("Min wed in the target is %lg mm.\n", total_min_wed);

    /* Save these values in private data store */
    // GCS FIX: To be revisited
    d_ptr->max_wed = total_max_wed;
    d_ptr->min_wed = total_min_wed;
}

void
Rt_beam::compute_target_wepl_min_max (
    std::vector<double>& map_wed_min,
    std::vector<double>& map_wed_max)
{
    Rpl_volume *wepl_rv = this->rsp_accum_vol;
    Volume *wepl_vol = wepl_rv->get_vol ();
    float *wepl_img = wepl_vol->get_raw<float> ();
    Rpl_volume::Pointer target_rv = this->target_rv;
    Volume *target_vol = target_rv->get_vol ();
    float *target_img = target_vol->get_raw<float> ();
    const plm_long *target_dim = target_vol->get_dim ();
    
    map_wed_min.resize (target_dim[0]*target_dim[1], NLMAX(double));
    map_wed_max.resize (target_dim[0]*target_dim[1], 0.);

    int ij[2] = {0, 0};
    int num_steps = this->target_rv->get_num_steps();
    for (ij[1] = 0; ij[1] < target_dim[1]; ij[1]++) {
        for (ij[0] = 0; ij[0] < target_dim[0]; ij[0]++) {
            int map_idx = ij[0] + ij[1] * target_dim[0];
            for (int s = 0; s < num_steps; s++) {
                int rv_index = target_vol->index (ij[0],ij[1],s);
                float tgt = target_img[rv_index];
                float wepl = wepl_img[rv_index];
                if (tgt < 0.2) {
                    continue;
                }
                if (map_wed_min[map_idx] > wepl) {
                    map_wed_min[map_idx] = wepl;
                }
                if (map_wed_max[map_idx] < wepl) {
                    map_wed_max[map_idx] = wepl;
                }
            }
        }
    }
}

void 
Rt_beam::apply_smearing_to_target (
    float smearing,
    std::vector <double>& map_min_distance,
    std::vector <double>& map_max_distance)
{
    // GCS FIX.  It appears that the reason for computing geometric
    // distance in the previous version of the code was to make the
    // smearing code act at the minimum target distance.  This is unnecessary; 
    // it is easier/better to apply at isocenter plane.

    // Convert smearing from isocenter to aperture plane
    const Aperture::Pointer& ap = d_ptr->aperture;
    float smearing_ap = smearing
        * ap->get_distance() / this->get_source_distance();
    printf ("Smearing = %f, Smearing_ap = %f\n", smearing, smearing_ap);

    /* Create a structured element of the right size */
    int strel_half_size[2];
    int strel_size[2];
    strel_half_size[0] = ROUND_INT (smearing_ap / ap->get_spacing()[0]);
    strel_half_size[1] = ROUND_INT (smearing_ap / ap->get_spacing()[1]);
    
    strel_size[0] = 1 + 2 * strel_half_size[0];
    strel_size[1] = 1 + 2 * strel_half_size[1];

    printf ("Strel size = (%d,%d), (%d,%d)\n", 
        strel_half_size[0], strel_half_size[1],
        strel_size[0], strel_size[1]);

    int *strel = new int[strel_size[0]*strel_size[1]];
    /* (rf, cf) center of the smearing */
    for (int r = 0; r < strel_size[1]; r++) {
        float rf = (float) (r - strel_half_size[1]) * ap->get_spacing()[0];
        for (int c = 0; c < strel_size[0]; c++) {
            float cf = (float) (c - strel_half_size[0]) * ap->get_spacing()[1];

            int idx = r*strel_size[0] + c;

            strel[idx] = 0;
            if ((rf*rf + cf*cf) <= smearing_ap*smearing_ap) {
                strel[idx] = 1;
            }
        }
    }

    /* Debugging information */
    for (int r = 0; r < strel_size[1]; r++) {
        for (int c = 0; c < strel_size[0]; c++) {
            int idx = r*strel_size[0] + c;
            printf ("%d ", strel[idx]);
        }
        printf ("\n");
    }

    // GCS FIX.  This is also in error.  The smearing should be done 
    // in WEPL rather than in geometric distance.
    /* Apply smear to target maps */
    double distance_min;
    double distance_max;
    std::vector<double> min_distance_tmp (map_min_distance.size(), 0);
    std::vector<double> max_distance_tmp (map_max_distance.size(), 0);

    for (int ar = 0; ar < d_ptr->aperture->get_dim()[1]; ar++) {
        for (int ac = 0; ac < d_ptr->aperture->get_dim()[0]; ac++) {
            int aidx = ar * d_ptr->aperture->get_dim()[0] + ac;

            /* Reset the limit values */
            distance_min = DBL_MAX;
            distance_max = 0;

            for (int sr = 0; sr < strel_size[1]; sr++) {
                int pr = ar + sr - strel_half_size[1];
                if (pr < 0 || pr >= d_ptr->aperture->get_dim()[1]) {
                    continue;
                }
                for (int sc = 0; sc < strel_size[0]; sc++) {
                    int pc = ac + sc - strel_half_size[0];
                    if (pc < 0 || pc >= d_ptr->aperture->get_dim()[0]) {
                        continue;
                    }

                    int sidx = sr * strel_size[0] + sc;
                    if (strel[sidx] == 0) {
                        continue;
                    }

                    int pidx = pr * d_ptr->aperture->get_dim()[0] + pc;
                    if (map_min_distance[pidx] > 0 && map_min_distance[pidx] < distance_min) {
                        distance_min = map_min_distance[pidx];
                    }
                    if (map_max_distance[pidx] > distance_max) {
                        distance_max = map_max_distance[pidx];
                    }
                }
            }
            if (distance_min == DBL_MAX)
            {
                min_distance_tmp[aidx] = 0;
            }
            else 
            {
                min_distance_tmp[aidx] = distance_min;
            }
            max_distance_tmp[aidx] = distance_max;
        }
    }

    /* update the initial distance map */
    for (size_t i = 0; i < map_min_distance.size(); i++) {
        map_min_distance[i] = min_distance_tmp[i];
        map_max_distance[i] = max_distance_tmp[i];
    }

    /* Clean up */
    delete[] strel;
}

void
Rt_beam::update_aperture_and_range_compensator ()
{
    // GCS FIX.  The below logic is no longer valid
#if defined (commentout)
    /* The aperture is copied from rpl_vol
       the range compensator and/or the aperture are erased if defined in the input file */
    if (d_ptr->aperture_in != "")
    {
        Plm_image::Pointer ap_img = Plm_image::New (d_ptr->aperture_in, PLM_IMG_TYPE_ITK_UCHAR);
        this->get_aperture()->set_aperture_image(d_ptr->aperture_in.c_str());
        this->get_aperture()->set_aperture_volume(ap_img->get_volume_uchar());
        if (this->rsp_accum_vol->get_minimum_distance_target() == 0) // means that there is no target defined
        {
            printf("Smearing applied to the aperture. The smearing width is defined in the aperture frame.\n");
            d_ptr->aperture->apply_smearing_to_aperture(d_ptr->smearing, d_ptr->aperture->get_distance());
        }
        else
        {
            printf("Smearing applied to the aperture. The smearing width is defined at the target minimal distance.\n");
            d_ptr->aperture->apply_smearing_to_aperture(d_ptr->smearing, this->rsp_accum_vol->get_minimum_distance_target());
        }
    }
    /* Set range compensator */
    if (d_ptr->range_compensator_in != "" && d_ptr->beam_line_type != "active")
    {
        Plm_image::Pointer rgc_img = Plm_image::New (d_ptr->range_compensator_in, PLM_IMG_TYPE_ITK_FLOAT);
        this->get_aperture()->set_range_compensator_image(d_ptr->range_compensator_in.c_str());
        this->get_aperture()->set_range_compensator_volume(rgc_img->get_volume_float());
		
        if (this->rsp_accum_vol->get_minimum_distance_target() == 0) // means that there is no target defined
        {
            printf("Smearing applied to the range compensator. The smearing width is defined in the aperture frame.\n");
            d_ptr->aperture->apply_smearing_to_range_compensator(d_ptr->smearing, d_ptr->aperture->get_distance());
        }
        else
        {
            printf("Smearing applied to the range compensator. The smearing width is defined at the target minimal distance.\n");
            d_ptr->aperture->apply_smearing_to_range_compensator(d_ptr->smearing, this->rsp_accum_vol->get_minimum_distance_target());
        }
    }
#endif
}

Plm_image::Pointer&
Rt_beam::get_ct_psp ()
{
    return d_ptr->ct_psp;
}

const Plm_image::Pointer&
Rt_beam::get_ct_psp () const 
{
    return d_ptr->ct_psp;
}

void 
Rt_beam::set_ct_psp (Plm_image::Pointer& ct_psp)
{
    d_ptr->ct_psp = ct_psp;
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
Rt_beam::set_target (Plm_image::Pointer& target)
{
    d_ptr->target = target;
}

void 
Rt_beam::set_rt_dose_timing (Rt_dose_timing::Pointer& rt_dose_timing)
{
    d_ptr->rt_dose_timing = rt_dose_timing;
}

Rt_dose_timing::Pointer& 
Rt_beam::get_rt_dose_timing ()
{
    return d_ptr->rt_dose_timing;
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

Plm_image::Pointer&
Rt_beam::get_aperture_image () 
{
    return d_ptr->aperture->get_aperture_image ();
}

const Plm_image::Pointer&
Rt_beam::get_aperture_image () const
{
    return d_ptr->aperture->get_aperture_image ();
}

Plm_image::Pointer&
Rt_beam::get_range_compensator_image () 
{
    return d_ptr->aperture->get_range_compensator_image ();
}

const Plm_image::Pointer&
Rt_beam::get_range_compensator_image () const
{
    return d_ptr->aperture->get_range_compensator_image ();
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
Rt_beam::set_step_length(float step)
{
    d_ptr->step_length = step;
}

float 
Rt_beam::get_step_length()
{
    return d_ptr->step_length;
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
Rt_beam::set_beam_dump_out(std::string str)
{
    d_ptr->beam_dump_out = str;
}

std::string 
Rt_beam::get_beam_dump_out()
{
    return d_ptr->beam_dump_out;
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
Rt_beam::set_proj_target_out(std::string str)
{
    d_ptr->proj_target_out = str;
}

std::string 
Rt_beam::get_proj_target_out()
{
    return d_ptr->proj_target_out;
}

void 
Rt_beam::set_beam_line_type(std::string str)
{
    if (str == "active")
    {
        d_ptr->beam_line_type = str;
    }
    else
    {
        d_ptr->beam_line_type = "passive";
    }
}

std::string
Rt_beam::get_beam_line_type()
{
    return d_ptr->beam_line_type;
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

bool
Rt_beam::get_intersection_with_aperture(double* idx_ap, int* idx, double* rest, double* ct_xyz)
{
    double ray[3] = {0,0,0};
    double length_on_normal_axis = 0;
	
    vec3_copy(ray, ct_xyz);
    vec3_sub2(ray, d_ptr->source);

    length_on_normal_axis = -vec3_dot(ray, hu_samp_vol->get_proj_volume()->get_nrm()); // MD Fix: why is the aperture not updated at this point? and why proj vol is?
    if (length_on_normal_axis < 0)
    {
        return false;
    }

    vec3_scale2(ray, this->get_aperture()->get_distance()/length_on_normal_axis);

    vec3_add2(ray, d_ptr->source);
    vec3_sub2(ray, hu_samp_vol->get_proj_volume()->get_ul_room());
					
    idx_ap[0] = vec3_dot(ray, hu_samp_vol->get_proj_volume()->get_incr_c()) / (this->get_aperture()->get_spacing(0) * this->get_aperture()->get_spacing(0));
    idx_ap[1] = vec3_dot(ray, hu_samp_vol->get_proj_volume()->get_incr_r()) / (this->get_aperture()->get_spacing(1) * this->get_aperture()->get_spacing(1));
    idx[0] = (int) floor(idx_ap[0]);
    idx[1] = (int) floor(idx_ap[1]);
    rest[0] = idx_ap[0] - (double) idx[0];
    rest[1] = idx_ap[1] - (double) idx[1];
    return true;
}

bool 
Rt_beam::is_ray_in_the_aperture(int* idx, unsigned char* ap_img)
{
    if ((float) ap_img[idx[0] + idx[1] * this->get_aperture()->get_dim(0)] == 0) {return false;}
    if (idx[0] + 1 < this->get_aperture()->get_dim(0))
    {
        if ((float) ap_img[idx[0] + 1 + idx[1] * this->get_aperture()->get_dim(0)] == 0) {return false;}
    }
    if (idx[1] + 1 < this->get_aperture()->get_dim(1))
    {
        if ((float) ap_img[idx[0] + (idx[1] + 1) * this->get_aperture()->get_dim(0)] == 0) {return false;}
    }
    if (idx[0] + 1 < this->get_aperture()->get_dim(0) && idx[1] + 1 < this->get_aperture()->get_dim(1))
    {
        if ((float) ap_img[idx[0] + 1 + (idx[1] + 1) * this->get_aperture()->get_dim(0)] == 0) {return false;}
    }
    return true;
}

float 
Rt_beam::compute_minimal_target_distance(Volume* target_vol, float background)
{
    float* target_img = (float*) target_vol->img;

    float min = FLT_MAX;
    int idx = 0;
    const plm_long *dim = target_vol->dim;
    float target_image_origin[3] = {target_vol->origin[0], target_vol->origin[1], target_vol->origin[2]};
    float target_image_spacing[3] = {target_vol->spacing[0], target_vol->spacing[1], target_vol->spacing[2]};
    float source[3] = {(float) this->get_source_position(0), (float) this->get_source_position(1), (float) this->get_source_position(2)};

    float voxel_xyz[3] = {0, 0, 0};
    float min_tmp;

    for (int k = 0; k < dim[2]; k++) 
    {
        for (int j = 0; j < dim[1]; j++) 
        {
            for (int i = 0; i < dim[0]; i++) 
            {
                idx = i + (dim[0] * (j + dim[1] * k));
                if (target_img[idx] > background)
                {
                    voxel_xyz[0] = target_image_origin[0] + (float) i * target_image_spacing[0];
                    voxel_xyz[1] = target_image_origin[1] + (float) j * target_image_spacing[1];
                    voxel_xyz[2] = target_image_origin[2] + (float) k * target_image_spacing[2];
                    min_tmp = vec3_dist(voxel_xyz, source);
                    if (min_tmp < min) {min = min_tmp;}
                }
            }
        }
    }
    return min;
}

void Rt_beam::set_energy_resolution (float eres)
{
    d_ptr->mebs->set_energy_resolution (eres);
}

float Rt_beam::get_energy_resolution () const
{
    return d_ptr->mebs->get_energy_resolution ();
}

void Rt_beam::set_proximal_margin (float proximal_margin)
{
    d_ptr->mebs->set_proximal_margin (proximal_margin);
}

float Rt_beam::get_proximal_margin () const
{
    return d_ptr->mebs->get_proximal_margin ();
}

void Rt_beam::set_distal_margin (float distal_margin)
{
    d_ptr->mebs->set_distal_margin (distal_margin);
}

float Rt_beam::get_distal_margin () const
{
    return d_ptr->mebs->get_distal_margin ();
}

void Rt_beam::set_prescription (float prescription_min, float prescription_max)
{
    d_ptr->mebs->set_prescription (prescription_min, prescription_max);
}

void
Rt_beam::save_beam_output ()
{
    /* Save beam modifiers */
    if (this->get_aperture_out() != "") {
        Rpl_volume *rpl_vol = this->rsp_accum_vol;
        Plm_image::Pointer& ap = rpl_vol->get_aperture()->get_aperture_image();
        ap->save_image (this->get_aperture_out().c_str());
    }

    if (this->get_range_compensator_out() != ""
        && this->get_beam_line_type() == "passive")
    {
        Rpl_volume *rpl_vol = this->rsp_accum_vol;
        Plm_image::Pointer& rc = rpl_vol->get_aperture()->get_range_compensator_image();
        rc->save_image (this->get_range_compensator_out().c_str());
    }

    /* Save projected density volume */
    if (d_ptr->proj_img_out != "") {
        Rpl_volume* proj_img = this->hu_samp_vol;
        if (proj_img) {
            proj_img->save (d_ptr->proj_img_out);
        }
    }

    /* Save projected dose volume */
    if (this->get_proj_dose_out() != "") {
        Rpl_volume* proj_dose = this->rpl_dose_vol;
        if (proj_dose) {
            proj_dose->save (this->get_proj_dose_out());
        }
    }

    /* Save wed volume */
    if (this->get_wed_out() != "") {
        Rpl_volume* rpl_vol = this->rsp_accum_vol;
        if (rpl_vol) {
            rpl_vol->save (this->get_wed_out());
        }
    }

    /* Save projected target volume */
    if (this->get_proj_target_out() != "") {
        Rpl_volume::Pointer rpl_vol = this->target_rv;
        if (rpl_vol) {
            rpl_vol->save (this->get_proj_target_out());
        }
    }

    /* Save the spot map */
    if (this->get_mebs()->get_particle_number_out() != "") {
        this->get_mebs()->export_spot_map_as_txt (this->get_aperture());
    }

    /* Dump beam information */
    if (this->get_beam_dump_out() != "") {
        this->dump (this->get_beam_dump_out());
    }
}
