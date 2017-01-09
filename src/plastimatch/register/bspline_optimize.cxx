/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmregister_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

#include "bspline.h"
#include "bspline_mi.h"
#include "bspline_optimize.h"
#include "bspline_optimize_lbfgsb.h"
#include "bspline_optimize_liblbfgs.h"
#include "bspline_optimize_nlopt.h"
#include "bspline_optimize_steepest.h"
#include "bspline_parms.h"
#include "bspline_state.h"
#include "bspline_xform.h"
#include "logfile.h"
#include "plm_math.h"

class Bspline_optimize_private 
{
public:
    Bspline_state::Pointer bst;
    Bspline_xform *bxf;
    Bspline_parms *parms;
};

Bspline_optimize::Bspline_optimize ()
{
    d_ptr = new Bspline_optimize_private;

    fixed = 0;
    moving = 0;
    moving_grad = 0;
}

Bspline_optimize::~Bspline_optimize ()
{
    delete d_ptr;
}

void 
Bspline_optimize::initialize (Bspline_xform *bxf, Bspline_parms *parms)
{
    d_ptr->parms = parms;
    d_ptr->bst = Bspline_state::New ();
    d_ptr->bxf = bxf;

    d_ptr->bst->initialize (bxf, parms);
}

static void
bspline_optimize_select (
    Bspline_optimize *bod
)
{
    Bspline_parms *parms = bod->get_bspline_parms ();

    switch (parms->optimization) {
    case BOPT_LBFGSB:
        bspline_optimize_lbfgsb (bod);
        break;
    case BOPT_STEEPEST:
        bspline_optimize_steepest (bod);
        break;
    case BOPT_LIBLBFGS:
        bspline_optimize_liblbfgs (bod);
        break;
#if (NLOPT_FOUND)
    case BOPT_NLOPT_LBFGS:
        bspline_optimize_nlopt (bod, NLOPT_LD_LBFGS);
        break;
    case BOPT_NLOPT_LD_MMA:
        bspline_optimize_nlopt (bod, NLOPT_LD_MMA);
        break;
    case BOPT_NLOPT_PTN_1:
        //bspline_optimize_nlopt (&bod, NLOPT_LD_TNEWTON_PRECOND_RESTART);
        bspline_optimize_nlopt (bod, NLOPT_LD_VAR2);
        break;
#else
    case BOPT_NLOPT_LBFGS:
    case BOPT_NLOPT_LD_MMA:
    case BOPT_NLOPT_PTN_1:
        logfile_printf (
            "Plastimatch was not compiled against NLopt.\n"
            "Reverting to liblbfgs.\n"
        );
        bspline_optimize_liblbfgs (bod);
#endif
    default:
        bspline_optimize_liblbfgs (bod);
        break;
    }
}

#if defined (commentout)
void
bspline_optimize (
    Bspline_xform *bxf, 
    Bspline_parms *parms
)
{
    Bspline_optimize bod;
    bod.initialize (bxf, parms);

    /* GCS FIX: The below does not belong in bspline_state.  And it should 
       be done if any similarity metric is MI. */
    /* JAS Fix 2011.09.14
     *   The MI algorithm will get stuck for a set of coefficients all equaling
     *   zero due to the method we use to compute the cost function gradient.
     *   However, it is possible we could be inheriting coefficients from a
     *   prior stage, so we must check for inherited coefficients before
     *   applying an initial offset to the coefficient array. */
    if (parms->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        bxf->jitter_if_zero ();
    }
    
    log_parms (parms);
    log_bxf_header (bxf);

    /* GCS FIX -- this should move into Bspline_state() constructor */
    /* Initialize histograms */
    /* GCS DOUBLE FIX -- normally this can be done once before the 
       first iteration.  But if e.g. there are two MI metrics, 
       we need two of these structures.  Perhaps it belongs in 
       Stage_similarity_data. */
    if (parms->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        bod.get_bspline_state()->mi_hist->initialize (
            parms->similarity_data.front()->fixed_ss.get(),
            parms->similarity_data.front()->moving_ss.get());
    }

    /* Do the optimization */
    bspline_optimize_select (&bod);
}
#endif

void
Bspline_optimize::optimize (
)
{
    Bspline_parms *parms = this->get_bspline_parms ();
    Bspline_state *bst = this->get_bspline_state ();
    Bspline_xform *bxf = this->get_bspline_xform ();
    
    /* GCS FIX: The below does not belong in bspline_state.  And it should 
       be done if any similarity metric is MI. */
    /* JAS Fix 2011.09.14
     *   The MI algorithm will get stuck for a set of coefficients all equaling
     *   zero due to the method we use to compute the cost function gradient.
     *   However, it is possible we could be inheriting coefficients from a
     *   prior stage, so we must check for inherited coefficients before
     *   applying an initial offset to the coefficient array. */
    if (parms->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        bxf->jitter_if_zero ();
    }

    parms->log ();
    bxf->log_header ();

    /* GCS FIX -- this should move into Bspline_state() constructor */
    /* Initialize histograms */
    /* GCS DOUBLE FIX -- normally this can be done once before the 
       first iteration.  But if e.g. there are two MI metrics, 
       we need two of these structures.  Perhaps it belongs in 
       Stage_similarity_data. */
    if (parms->has_metric_type (SIMILARITY_METRIC_MI_MATTES)) {
        bst->mi_hist->initialize (
            parms->similarity_data.front()->fixed_ss.get(),
            parms->similarity_data.front()->moving_ss.get());
    }

    /* Do the optimization */
    bspline_optimize_select (this);
}

Bspline_parms* 
Bspline_optimize::get_bspline_parms ()
{
    return d_ptr->parms;
}

Bspline_state* 
Bspline_optimize::get_bspline_state ()
{
    return d_ptr->bst.get();
}

Bspline_xform*
Bspline_optimize::get_bspline_xform ()
{
    return d_ptr->bxf;
}
