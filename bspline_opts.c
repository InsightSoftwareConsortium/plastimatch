/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plm_config.h"
#include "bspline_opts.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

void
print_usage (void)
{
    printf (
	"Usage: bspline [options] fixed moving\n"
	"Options:\n"
	" -A hardware                Either \"cpu\" or \"cuda\" (default=cpu)\n"
	" -a { steepest | lbfgsb }   Choose optimization algorithm\n"
	" -M { mse | mi }            Registration metric (default is mse)\n"
	" -f implementation          Choose implementation (a single letter: a, b, etc.)\n"
	" -m iterations              Maximum iterations (default is 10)\n"
	" --factr value              L-BFGS-B cost converg tol (default is 1e+7)\n"
	" --pgtol value              L-BFGS-B projected grad tol (default is 1e-5)\n"
	" -s \"i j k\"                 Integer knot spacing (voxels)\n"
	" -h prefix                  Generate histograms for each MI iteration\n"
	" -V outfile                 Output vector field\n"
	" -X infile                  Input bspline coefficients\n"
	" -x outfile                 Output bspline coefficients\n"
	" -O outfile                 Output warped image\n"
	" --debug                    Create various debug files\n"
    );
    exit (1);
}

void
bspline_opts_parse_args (BSPLINE_Options* options, int argc, char* argv[])
{
    int d, i, rc;
    BSPLINE_Parms* parms = &options->parms;

    memset (options, 0, sizeof (BSPLINE_Options));
    for (d = 0; d < 3; d++) {
	options->vox_per_rgn[d] = 15;
    }
    bspline_parms_set_default (parms);

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') break;
	if (!strcmp (argv[i], "-A")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "cuda") || !strcmp(argv[i], "CUDA")
		|| !strcmp(argv[i], "gpu") || !strcmp(argv[i], "GPU")) {
		parms->threading = BTHR_CUDA;
	    }
	    else {
		parms->threading = BTHR_CPU;
	    }
	}
        else if (!strcmp (argv[i], "-a")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "steepest")) {
		parms->optimization = BOPT_STEEPEST;
	    } else if (!strcmp(argv[i], "nlopt-lbfgs")) {
		parms->optimization = BOPT_NLOPT_LBFGS;
	    } else if (!strcmp(argv[i], "nlopt-ld-mma")) {
		parms->optimization = BOPT_NLOPT_LD_MMA;
	    } else if (!strcmp(argv[i], "nlopt-ptn")) {
		parms->optimization = BOPT_NLOPT_PTN_1;
	    } else if (!strcmp(argv[i], "lbfgsb")) {
		parms->optimization = BOPT_LBFGSB;
	    } else {
		fprintf (stderr, "Unknown optimization type: %s\n", argv[i]);
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-f")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->implementation = argv[i][0];
	}
	else if (!strcmp (argv[i], "-m")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%d" , &parms->max_its);
	    if (rc != 1) {
		print_usage ();
	    }
	}
	else if (!strcmp (argv[i], "-M")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    if (!strcmp(argv[i], "mse")) {
		parms->metric = BMET_MSE;
	    } else if (!strcmp(argv[i], "mi")) {
		parms->metric = BMET_MI;
	    } else {
		print_usage ();
	    }
	}
        else if (!strcmp (argv[i], "-s")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%d %d %d", 
		&options->vox_per_rgn[0],
		&options->vox_per_rgn[1],
		&options->vox_per_rgn[2]);
	    if (rc == 1) {
		options->vox_per_rgn[1] = options->vox_per_rgn[0];
		options->vox_per_rgn[2] = options->vox_per_rgn[0];
	    } else if (rc != 3) {
		print_usage ();
	    }
	}
        else if (!strcmp (argv[i], "-h")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    parms->xpm_hist_dump = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "-O")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_warped_fn = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "-V")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_vf_fn = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "-x")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->output_xf_fn = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "-X")) {
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    options->input_xf_fn = strdup (argv[i]);
	}
        else if (!strcmp (argv[i], "--debug")) {
	    parms->debug = 1;
	}
        else if (!strcmp (argv[i], "--factr")) {
	    float f;
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &f);
	    if (rc != 1) {
		print_usage ();
	    }
	    parms->lbfgsb_factr = (double) f;
	}
        else if (!strcmp (argv[i], "--pgtol")) {
	    float f;
	    if (i == (argc-1) || argv[i+1][0] == '-') {
		fprintf(stderr, "option %s requires an argument\n", argv[i]);
		exit(1);
	    }
	    i++;
	    rc = sscanf (argv[i], "%g", &f);
	    if (rc != 1) {
		print_usage ();
	    }
	    parms->lbfgsb_pgtol = (double) f;
	}
	else {
	    print_usage ();
	    break;
	}
    }
    if (i+1 >= argc) {
	print_usage ();
    }
    options->fixed_fn = argv[i];
    options->moving_fn = argv[i+1];
    printf ("Fixed = %s\n", options->fixed_fn);
    printf ("Moving = %s\n", options->moving_fn);
}
