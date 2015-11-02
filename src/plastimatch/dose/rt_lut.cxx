/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"

#include "rt_lut.h"

double get_proton_range(double energy)
{
    int i_lo = 0, i_hi = PROTON_TABLE_SIZE;
    double energy_lo = lookup_proton_range_water[i_lo][0];
    double range_lo = lookup_proton_range_water[i_lo][1];
    double energy_hi = lookup_proton_range_water[i_hi][0];
    double range_hi = lookup_proton_range_water[i_hi][1];

    if (energy <= energy_lo) {
        return range_lo;
    }
    if (energy >= energy_hi) {
        return range_hi;
    }

    /* Use binary search to find lookup table entries */
    for (int dif = i_hi - i_lo; dif > 1; dif = i_hi - i_lo) {
        int i_test = i_lo + ((dif + 1) / 2);
        double energy_test = lookup_proton_range_water[i_test][0];
        if (energy > energy_test) {
            energy_lo = energy_test;
            i_lo = i_test;
        } else {
            energy_hi = energy_test;
            i_hi = i_test;
        }
    }

    range_lo = lookup_proton_range_water[i_lo][1];
    range_hi = lookup_proton_range_water[i_hi][1];
    return range_lo + 
        (energy-energy_lo) * (range_hi-range_lo) / (energy_hi-energy_lo);
}

double get_proton_stop (double energy)
{
    int i_lo = 0, i_hi = PROTON_TABLE_SIZE;
    double energy_lo = lookup_proton_stop_water[i_lo][0];
    double stop_lo = lookup_proton_stop_water[i_lo][1];
    double energy_hi = lookup_proton_stop_water[i_hi][0];
    double stop_hi = lookup_proton_stop_water[i_hi][1];

    if (energy <= energy_lo) {
        return stop_lo;
    }
    if (energy >= energy_hi) {
        return stop_hi;
    }

    /* Use binary search to find lookup table entries */
    for (int dif = i_hi - i_lo; dif > 1; dif = i_hi - i_lo) {
        int i_test = i_lo + ((dif + 1) / 2);
        double energy_test = lookup_proton_stop_water[i_test][0];
        if (energy > energy_test) {
            energy_lo = energy_test;
            i_lo = i_test;
        } else {
            energy_hi = energy_test;
            i_hi = i_test;
        }
    }

    stop_lo = lookup_proton_stop_water[i_lo][1];
    stop_hi = lookup_proton_stop_water[i_hi][1];
    return stop_lo + 
        (energy-energy_lo) * (stop_hi-stop_lo) / (energy_hi-energy_lo);
}

double get_proton_dose_max(double E0)
{
    if (E0 < 0 || E0 > PROTON_E_MAX)
    {
        return 1;
    }

    int E0_floor = floor(E0);
    double rest = E0 - (double) E0_floor;
		
    return lookup_proton_dose_max_bragg[E0_floor][0] + rest * (lookup_proton_dose_max_bragg[E0_floor+1][0]-lookup_proton_dose_max_bragg[E0_floor][0]);
}

int get_proton_depth_max(int E0)
{
    if (E0 < 0)
    {
        return 0;
    }
    else if (E0 >PROTON_E_MAX)
    {
        return 40000;
    }
    else
    {
        return lookup_proton_dose_max_bragg[E0][1];
    }
}

double get_theta0_Highland(double range)
{
	/* lucite sigma0 (in rads) computing- From the figure A2 of the Hong's paper (be careful, in this paper the fit shows sigma0^2)*/
	if (range > 150)
	{
		return 0.05464 + 5.8348E-6 * range -5.21006E-9 * range * range;
	}
	else 
	{
		return 0.05394 + 1.80222E-5 * range -5.5430E-8 * range * range;
	}
}

double get_theta0_MC(float energy)
{
	return 4.742E-6 * energy * energy -1.918E-3 * energy + 1.158;
}

double get_theta_rel_Highland(double rc_over_range)
{
	return rc_over_range * ( 1.6047 -2.7295 * rc_over_range + 2.1578 * rc_over_range * rc_over_range);
}

double get_theta_rel_MC(double rc_over_range)
{
	return 3.833E-2 * pow(rc_over_range, 0.657) + 2.118E-2 * pow(rc_over_range, 6.528);
}

double get_scat_or_Highland(double rc_over_range)
{
	/* calculation of rc_eff - see Hong's paper graph A3 - linear interpolation of the curve */
	if (rc_over_range >= 0 && rc_over_range < 0.5)
    {
        return 1 - (0.49 + 0.060 / 0.5 * rc_over_range);
    }
    else if (rc_over_range >= 0.5 && rc_over_range <0.8)
    {
        return 1 - (0.55 + 0.085 / 0.3 * (rc_over_range-0.5));
    }
    else if (rc_over_range >= 0.8 && rc_over_range <0.9)
    {
        return 1 - (0.635 + 0.055 / 0.1 * (rc_over_range-0.8));
    }
    else if (rc_over_range >= 0.9 && rc_over_range <0.95)
    {
        return 1 - (0.690 + (rc_over_range-0.9));
    }
    else if (rc_over_range >= 0.95 && rc_over_range <= 1)
    {
        return 1 - (0.740 + 0.26/0.05 * (rc_over_range-0.95));
    }
    else if (rc_over_range > 1)
    {
        return 0;
    }
    else
    {
        return 0;
    }
}

double get_scat_or_MC(double rc_over_range)
{
	return 0.023 * rc_over_range + 0.332;
}

double compute_X0_from_HU(double CT_HU)
{
    if (CT_HU <= -1000)
    {
        return 30390;
    }
    else if (CT_HU > -1000 && CT_HU< 0)
    {
        return exp(3.7271E-06 * CT_HU * CT_HU -3.009E-03 * CT_HU + 3.5857);
    }
    else if (CT_HU >= 0 && CT_HU < 55)
    {
        return -0.0284 * CT_HU + 36.08;
    }
    else
    {
        return 9.8027E-06 * CT_HU * CT_HU -.028939 * CT_HU + 36.08;
    }
}

/* Make GCC compiler less whiny */
#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2)
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

/* This table has 111 entries */
const double lookup_proton_range_water[][2] ={
1.000E-03,	6.319E-06,
1.500E-03,	8.969E-06,	
2.000E-03,	1.137E-05,	
2.500E-03,	1.357E-05,	
3.000E-03,	1.560E-05,	
4.000E-03,	1.930E-05,	
5.000E-03,	2.262E-05,	
6.000E-03,	2.567E-05,	
7.000E-03,	2.849E-05,	
8.000E-03,	3.113E-05,	
9.000E-03,	3.363E-05,	
1.000E-02,	3.599E-05,	
1.250E-02,	4.150E-05,	
1.500E-02,	4.657E-05,	
1.750E-02,	5.131E-05,	
2.000E-02,	5.578E-05,	
2.250E-02,	6.005E-05,	
2.500E-02,	6.413E-05,	
2.750E-02,	6.806E-05,	
3.000E-02,	7.187E-05,	
3.500E-02,	7.916E-05,	
4.000E-02,	8.613E-05,	
4.500E-02,	9.284E-05,	
5.000E-02,	9.935E-05,	
5.500E-02,	1.057E-04,	
6.000E-02,	1.120E-04,	
6.500E-02,	1.182E-04,	
7.000E-02,	1.243E-04,	
7.500E-02,	1.303E-04,	
8.000E-02,	1.364E-04,	
8.500E-02,	1.425E-04,	
9.000E-02,	1.485E-04,	
9.500E-02,	1.546E-04,	
1.000E-01,	1.607E-04,	
1.250E-01,	1.920E-04,	
1.500E-01,	2.249E-04,	
1.750E-01,	2.598E-04,	
2.000E-01,	2.966E-04,	
2.250E-01,	3.354E-04,	
2.500E-01,	3.761E-04,	
2.750E-01,	4.186E-04,	
3.000E-01,	4.631E-04,	
3.500E-01,	5.577E-04,	
4.000E-01,	6.599E-04,	
4.500E-01,	7.697E-04,	
5.000E-01,	8.869E-04,	
5.500E-01,	1.012E-03,	
6.000E-01,	1.144E-03,	
6.500E-01,	1.283E-03,	
7.000E-01,	1.430E-03,	
7.500E-01,	1.584E-03,	
8.000E-01,	1.745E-03,	
8.500E-01,	1.913E-03,	
9.000E-01,	2.088E-03,	
9.500E-01,	2.270E-03,	
1.000E+00,	2.458E-03,	
1.250E+00,	3.499E-03,	
1.500E+00,	4.698E-03,	
1.750E+00,	6.052E-03,	
2.000E+00,	7.555E-03,	
2.250E+00,	9.203E-03,	
2.500E+00,	1.099E-02,	
2.750E+00,	1.292E-02,	
3.000E+00,	1.499E-02,	
3.500E+00,	1.952E-02,	
4.000E+00,	2.458E-02,	
4.500E+00,	3.015E-02,	
5.000E+00,	3.623E-02,	
5.500E+00,	4.279E-02,	
6.000E+00,	4.984E-02,	
6.500E+00,	5.737E-02,	
7.000E+00,	6.537E-02,	
7.500E+00,	7.384E-02,	
8.000E+00,	8.277E-02,	
8.500E+00,	9.215E-02,	
9.000E+00,	1.020E-01,	
9.500E+00,	1.123E-01,	
1.000E+01,	1.230E-01,	
1.250E+01,	1.832E-01,	
1.500E+01,	2.539E-01,	
1.750E+01,	3.350E-01,	
2.000E+01,	4.260E-01,	
2.500E+01,	6.370E-01,	
2.750E+01,	7.566E-01,	
3.000E+01,	8.853E-01,	
3.500E+01,	1.170E+00,	
4.000E+01,	1.489E+00,	
4.500E+01,	1.841E+00,	
5.000E+01,	2.227E+00,	
5.500E+01,	2.644E+00,	
6.000E+01,	3.093E+00,	
6.500E+01,	3.572E+00,	
7.000E+01,	4.080E+00,	
7.500E+01,	4.618E+00,	
8.000E+01,	5.184E+00,	
8.500E+01,	5.777E+00,	
9.000E+01,	6.398E+00,	
9.500E+01,	7.045E+00,	
1.000E+02,	7.718E+00,	
1.250E+02,	1.146E+01,	
1.500E+02,	1.577E+01,	
1.750E+02,	2.062E+01,	
2.000E+02,	2.596E+01,	
2.250E+02,	3.174E+01,	
2.500E+02,	3.794E+01,	
2.750E+02,	4.452E+01,	
3.000E+02,	5.145E+01,	
3.500E+02,	6.628E+01,	
4.000E+02,	8.225E+01,	
4.500E+02,	9.921E+01,	
5.000E+02,	1.170E+02,	
};

/* This table has 111 entries */
const double lookup_proton_stop_water[][2] =
{
0.0010,	176.9,
0.0015,	198.4,
0.0020,	218.4,
0.0025,	237.0,
0.0030,	254.4,
0.0040,	286.4,
0.0050,	315.3,
0.0060,	342.0,
0.0070,	366.7,
0.0080,	390.0,
0.0090,	412.0,
0.0100,	432.9,
0.0125,	474.5,
0.0150,	511.0,
0.0175,	543.7,
0.0200,	573.3,
0.0225,	600.1,
0.0250,	624.5,
0.0275,	646.7,
0.0300,	667.1,
0.0350,	702.8,
0.0400,	732.4,
0.0450,	756.9,
0.0500,	776.8,
0.0550,	792.7,
0.0600,	805.0,
0.0650,	814.2,
0.0700,	820.5,
0.0750,	824.3,
0.0800,	826.0,
0.0850,	825.8,
0.0900,	823.9,
0.0950,	820.6,
0.1000,	816.1,
0.1250,	781.4,
0.1500,	737.1,
0.1750,	696.9,
0.2000,	661.3,
0.2250,	629.4,
0.2500,	600.6,
0.2750,	574.4,
0.3000,	550.4,
0.3500,	508.0,
0.4000,	471.9,
0.4500,	440.6,
0.5000,	413.2,
0.5500,	389.1,
0.6000,	368.0,
0.6500,	349.2,
0.7000,	332.5,
0.7500,	317.5,
0.8000,	303.9,
0.8500,	291.7,
0.9000,	280.5,
0.9500,	270.2,
1.0000,	260.8,
1.2500,	222.9,
1.5000,	195.7,
1.7500,	174.9,
2.0000,	158.6,
2.2500,	145.4,
2.5000,	134.4,
2.7500,	125.1,
3.0000,	117.2,
3.5000,	104.2,
4.0000,	94.04,
4.5000,	85.86,
5.0000,	79.11,
5.5000,	73.43,
6.0000,	68.58,
6.5000,	64.38,
7.0000,	60.71,
7.5000,	57.47,
8.0000,	54.60,
8.5000,	52.02,
9.0000,	49.69,
9.5000,	47.59,
10.000,	45.67,
12.500,	38.15,
15.000,	32.92,
17.500,	29.05,
20.000,	26.07,
25.000,	21.75,
27.500,	20.13,
30.000,	18.76,
35.000,	16.56,
40.000,	14.88,
45.000,	13.54,
50.000,	12.45,
55.000,	11.54,
60.000,	10.78,
65.000,	10.13,
70.000,	9.559,
75.000,	9.063,
80.000,	8.625,
85.000,	8.236,
90.000,	7.888,
95.000,	7.573,
100.00,	7.289,
125.00,	6.192,
150.00,	5.445,
175.00,	4.903,
200.00,	4.492,
225.00,	4.170,
250.00,	3.911,
275.00,	3.698,
300.00,	3.520,
350.00,	3.241,
400.00,	3.032,
450.00,	2.871,
500.00,	2.743,
};

extern const double lookup_proton_dose_max_bragg[][2] = { // [0]: dose max, [1]: depth max in 0.01mm
230.1791,	0, // first line E = 0, extrapolation to 0 MeV
230.1791,	0,
220.9350,	4,
219.9710,	13,
223.9043,	23,
226.3141,	35,
227.6480,	50,
227.6221,	66,
226.1386,	85,
223.2419,	105,
219.0823,	127,
213.9347,	151,
208.0689,	176,
201.7980,	203,
194.8930,	232,
188.8440,	262,
182.3840,	294,
176.4200,	327,
170.5880,	362,
164.7550,	399,
159.5390,	436,
154.7960,	476,
150.1000,	517,
145.6560,	559,
141.5270,	603,
137.5560,	648,
133.8990,	695,
130.3960,	743,
127.0300,	793,
123.9160,	843,
120.9480,	896,
118.1550,	949,
115.4790,	1004,
112.9110,	1060,
110.4990,	1118,
108.1800,	1177,
105.9680,	1237,
103.8380,	1299,
101.8120,	1361,
99.8712,	1426,
98.0264,	1491,
96.2303,	1558,
94.5115,	1626,
92.8698,	1695,
91.2850,	1765,
89.7541,	1837,
88.2773,	1910,
86.8559,	1984,
85.4793,	2059,
84.1527,	2136,
82.8659,	2214,
81.6235,	2293,
80.4220,	2373,
79.2532,	2454,
78.1254,	2537,
77.0279,	2621,
75.9635,	2706,
74.9307,	2792,
73.9252,	2879,
72.9495,	2968,
71.9976,	3057,
71.0753,	3148,
70.1759,	3240,
69.2986,	3333,
68.4462,	3428,
67.6145,	3523,
66.8025,	3620,
66.0098,	3717,
65.2381,	3816,
64.4838,	3916,
63.7465,	4017,
63.0258,	4120,
62.3234,	4223,
61.6351,	4327,
60.9625,	4433,
60.3032,	4539,
59.6599,	4647,
59.0294,	4756,
58.4119,	4866,
57.8070,	4977,
57.2142,	5089,
56.6329,	5202,
56.0637,	5317,
55.5057,	5432,
54.9573,	5549,
54.4209,	5666,
53.8933,	5785,
53.3761,	5904,
52.8684,	6025,
52.3695,	6147,
51.8795,	6270,
51.3981,	6394,
50.9252,	6519,
50.4605,	6645,
50.0039,	6772,
49.5550,	6900,
49.1135,	7029,
48.6792,	7159,
48.2518,	7290,
47.8313,	7423,
47.4180,	7556,
47.0109,	7690,
46.6099,	7826,
46.2156,	7962,
45.8270,	8099,
45.4445,	8238,
45.0676,	8377,
44.6963,	8518,
44.3305,	8659,
43.9701,	8802,
43.6148,	8945,
43.2646,	9090,
42.9194,	9235,
42.5791,	9382,
42.2433,	9529,
41.9125,	9678,
41.5858,	9827,
41.2639,	9978,
40.9460,	10130,
40.6326,	10282,
40.3231,	10436,
40.0179,	10590,
39.7165,	10746,
39.4191,	10902,
39.1252,	11060,
38.8354,	11218,
38.5489,	11377,
38.2663,	11538,
37.9870,	11699,
37.7110,	11862,
37.4386,	12025,
37.1694,	12189,
36.9033,	12354,
36.6406,	12521,
36.3809,	12688,
36.1242,	12856,
35.8705,	13025,
35.6197,	13195,
35.3717,	13366,
35.1267,	13538,
34.8844,	13711,
34.6448,	13885,
34.4079,	14060,
34.1736,	14236,
33.9419,	14413,
33.7126,	14591,
33.4860,	14769,
33.2617,	14949,
33.0398,	15129,
32.8204,	15311,
32.6032,	15493,
32.3883,	15677,
32.1756,	15861,
31.9652,	16046,
31.7568,	16233,
31.5507,	16420,
31.3467,	16608,
31.1447,	16797,
30.9448,	16987,
30.7468,	17178,
30.5509,	17369,
30.3568,	17562,
30.1647,	17756,
29.9745,	17950,
29.7861,	18146,
29.5995,	18342,
29.4147,	18539,
29.2317,	18738,
29.0505,	18937,
28.8709,	19137,
28.6931,	19338,
28.5169,	19540,
28.3423,	19743,
28.1694,	19946,
27.9981,	20151,
27.8283,	20357,
27.6601,	20563,
27.4934,	20770,
27.3282,	20979,
27.1645,	21188,
27.0023,	21398,
26.8416,	21609,
26.6822,	21821,
26.5243,	22034,
26.3677,	22247,
26.2126,	22462,
26.0587,	22677,
25.9062,	22894,
25.7551,	23111,
25.6052,	23329,
25.4566,	23548,
25.3093,	23768,
25.1633,	23989,
25.0184,	24210,
24.8748,	24433,
24.7324,	24657,
24.5912,	24881,
24.4511,	25106,
24.3122,	25332,
24.1745,	25559,
24.0379,	25787,
23.9024,	26016,
23.7680,	26246,
23.6347,	26476,
23.5025,	26707,
23.3714,	26940,
23.2413,	27173,
23.1122,	27407,
22.9842,	27642,
22.8572,	27878,
22.7312,	28114,
22.6062,	28352,
22.4821,	28590,
22.3591,	28829,
22.2370,	29070,
22.1158,	29311,
21.9956,	29552,
21.8763,	29795,
21.7579,	30039,
21.6405,	30283,
21.5239,	30528,
21.4082,	30775,
21.2934,	31022,
21.1795,	31270,
21.0664,	31518,
20.9542,	31768,
20.8428,	32018,
20.7323,	32270,
20.6226,	32522,
20.5137,	32775,
20.4056,	33029,
20.2983,	33283,
20.1917,	33539,
20.0860,	33795,
19.9810,	34053,
19.8769,	34311,
19.7734,	34570,
19.6707,	34830,
19.5688,	35090,
19.4676,	35352,
19.3671,	35614,
19.2673,	35877,
19.1683,	36141,
19.0699,	36406,
18.9723,	36672,
18.8753,	36939,
18.7790,	37206,
18.6834,	37474,
18.5885,	37743,
18.4943,	38013,
18.4007,	38284,
18.3077,	38556,
18.2154,	38828,
18.1238,	39101,
18.0328,	39375,
17.9424,	39650,
};

/* matrix that contains the alpha and p parameters from equation: Range = f(E, alpha, p)
    First line is proton, second line is He... */

extern const double particle_parameters[][2] = {
    0.0022, 1.77,   //P
	/* To be updated to the right values for ions */
    0.0022, 1.77,   //HE
    0.0022, 1.77,   //LI
    0.0022, 1.77,   //BE
    0.0022, 1.77,   //B
    0.0022, 1.77,   //C
    0.00,   0.00,   //N not used for ion therapy - set to 0
    0.0022, 1.77    //O
};

extern const int max_depth_proton[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	3,
	3,
	3,
	4,
	4,
	4,
	5,
	5,
	6,
	6,
	7,
	7,
	8,
	8,
	9,
	9,
	10,
	10,
	11,
	12,
	12,
	13,
	14,
	14,
	15,
	15,
	16,
	17,
	17,
	18,
	19,
	20,
	20,
	21,
	22,
	23,
	24,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	49,
	50,
	51,
	52,
	53,
	54,
	55,	
	57,
	58,	
	59,
	60,
	61,
	63,
	64,
	65,
	66,
	68,
	69,
	70,
	72,
	73,
	74,
	76,
	77,
	78,
	80,
	81,
	82,
	84,
	85,	
	87,
	88,
	89,
	91,
	92,
	94,
	95,
	97,
	98,
	100,
	101,
	103,
	104,
	106,
	107,
	109,
	111,
	112,
	114,
	115,
	117,
	119,
	120,
	122,
	124,
	125,
	127,
	129,
	130,
	132,
	134,
	135,
	137,
	139,
	141,
	142,
	144,
	146,
	148,	
	149,
	151,
	153,
	155,
	157,
	159,
	160,
	162,
	164,
	166,
	168,
	170,
	172,
	174,
	176,
	178,
	179,
	181,
	183,
	185,
	187,
	189,
	191,
	193,
	195,
	197,
	199,
	201,
	204,
	206,
	208,
	210,
	212,
	214,
	216,
	218,
	220,
	222,
	225,
	227,
	229,
	231,
	233,
	235,
	238,
	240,
	242,
	244,
	247,
	249,
	251,
	253,
	256,
	258,
};
