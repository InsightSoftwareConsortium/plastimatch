/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plm_config.h"
#include <time.h>
#include "itkImageRegionIterator.h"
#include "getopt.h"
#include "stats_main.h"
#include "itk_image.h"

static void
stats_main (Stats_parms* parms)
{
    typedef itk::ImageRegionIterator< FloatImageType > FloatIteratorType;
    FloatImageType::Pointer img = load_float (parms->mha_in_fn, 0);
    FloatImageType::RegionType rg = img->GetLargestPossibleRegion ();
    FloatIteratorType it (img, rg);

    int first = 1;
    float min_val, max_val;
    int num = 0;
    double sum = 0.0;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
	float v = it.Get();
	if (first) {
	    min_val = max_val = v;
	    first = 0;
	}
	if (min_val > v) min_val = v;
	if (max_val < v) max_val = v;
	sum += v;
	num ++;
    }

    printf ("MIN %f AVE %f MAX %f NUM %d\n",
	    min_val, max_val, (float) (sum / num), num);
}

static void
stats_print_usage (void)
{
    printf ("Usage: plastimatch stats [options]\n"
	    "Required:\n"
	    "    --input=image_in\n"
	    );
    exit (-1);
}

static void
stats_parse_args (Stats_parms* parms, int argc, char* argv[])
{
    int ch;
    static struct option longopts[] = {
	{ "input",          required_argument,      NULL,           2 },
	{ NULL,             0,                      NULL,           0 }
    };

    while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
	switch (ch) {
	case 2:
	    strncpy (parms->mha_in_fn, optarg, _MAX_PATH);
	    break;
	default:
	    break;
	}
    }
    if (!parms->mha_in_fn[0]) {
	printf ("Error: must specify --input and --output\n");
	stats_print_usage ();
    }
}

void
do_command_stats (int argc, char *argv[])
{
    Stats_parms parms;
    
    stats_parse_args (&parms, argc, argv);

    stats_main (&parms);
}
