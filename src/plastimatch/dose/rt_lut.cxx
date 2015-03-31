/* -----------------------------------------------------------------------
   See COPYRIGHT.TXT and LICENSE.TXT for copyright and license information
   ----------------------------------------------------------------------- */
#include "plmdose_config.h"

#include "rt_lut.h"

double getrange(double energy)
{
    int i_lo = 0, i_hi = 110;
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

double getstop (double energy)
{
    int i_lo = 0, i_hi = 110;
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

double get_dose_max(double E0)
{
	if (E0 < 0 || E0 > 255)
	{
		return 1;
	}

	int E0_floor = floor(E0);
	double rest = E0 - (double) E0_floor;
		
	return lookup_proton_dose_max_bragg[E0_floor][0] + rest * (lookup_proton_dose_max_bragg[E0_floor+1][0]-lookup_proton_dose_max_bragg[E0_floor][0]);
}

int get_depth_max(double E0)
{
	int E0_floor = floorf(E0); 
	if (E0 < 0)
	{
		return 0;
	}
	else if (E0 >255)
	{
		return 40000;
	}
	else
	{
		return lookup_proton_dose_max_bragg[E0_floor][1];
	}

}

/* Make GCC compiler less whiny */
#if (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 2)
#pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

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

const double lookup_off_axis[][2]={ // x[0] = r/sigma, x[1] = exp(-x�/(2 * sigma�))
0.00, 1.0000,
0.01, 1.0000,
0.02, 0.9998,
0.03, 0.9996,
0.04, 0.9992,
0.05, 0.9988,
0.06, 0.9982,
0.07, 0.9976,
0.08, 0.9968,
0.09, 0.9960,
0.10, 0.9950,
0.11, 0.9940,
0.12, 0.9928,
0.13, 0.9916,
0.14, 0.9902,
0.15, 0.9888,
0.16, 0.9873,
0.17, 0.9857,
0.18, 0.9839,
0.19, 0.9821,
0.20, 0.9802,
0.21, 0.9782,
0.22, 0.9761,
0.23, 0.9739,
0.24, 0.9716,
0.25, 0.9692,
0.26, 0.9668,
0.27, 0.9642,
0.28, 0.9616,
0.29, 0.9588,
0.30, 0.9560,
0.31, 0.9531,
0.32, 0.9501,
0.33, 0.9470,
0.34, 0.9438,
0.35, 0.9406,
0.36, 0.9373,
0.37, 0.9338,
0.38, 0.9303,
0.39, 0.9268,
0.40, 0.9231,
0.41, 0.9194,
0.42, 0.9156,
0.43, 0.9117,
0.44, 0.9077,
0.45, 0.9037,
0.46, 0.8996,
0.47, 0.8954,
0.48, 0.8912,
0.49, 0.8869,
0.50, 0.8825,
0.51, 0.8781,
0.52, 0.8735,
0.53, 0.8690,
0.54, 0.8643,
0.55, 0.8596,
0.56, 0.8549,
0.57, 0.8501,
0.58, 0.8452,
0.59, 0.8403,
0.60, 0.8353,
0.61, 0.8302,
0.62, 0.8251,
0.63, 0.8200,
0.64, 0.8148,
0.65, 0.8096,
0.66, 0.8043,
0.67, 0.7990,
0.68, 0.7936,
0.69, 0.7882,
0.70, 0.7827,
0.71, 0.7772,
0.72, 0.7717,
0.73, 0.7661,
0.74, 0.7605,
0.75, 0.7548,
0.76, 0.7492,
0.77, 0.7435,
0.78, 0.7377,
0.79, 0.7319,
0.80, 0.7261,
0.81, 0.7203,
0.82, 0.7145,
0.83, 0.7086,
0.84, 0.7027,
0.85, 0.6968,
0.86, 0.6909,
0.87, 0.6849,
0.88, 0.6790,
0.89, 0.6730,
0.90, 0.6670,
0.91, 0.6610,
0.92, 0.6549,
0.93, 0.6489,
0.94, 0.6429,
0.95, 0.6368,
0.96, 0.6308,
0.97, 0.6247,
0.98, 0.6187,
0.99, 0.6126,
1.00, 0.6065,
1.01, 0.6005,
1.02, 0.5944,
1.03, 0.5883,
1.04, 0.5823,
1.05, 0.5762,
1.06, 0.5702,
1.07, 0.5641,
1.08, 0.5581,
1.09, 0.5521,
1.10, 0.5461,
1.11, 0.5401,
1.12, 0.5341,
1.13, 0.5281,
1.14, 0.5222,
1.15, 0.5162,
1.16, 0.5103,
1.17, 0.5044,
1.18, 0.4985,
1.19, 0.4926,
1.20, 0.4868,
1.21, 0.4809,
1.22, 0.4751,
1.23, 0.4693,
1.24, 0.4636,
1.25, 0.4578,
1.26, 0.4521,
1.27, 0.4464,
1.28, 0.4408,
1.29, 0.4352,
1.30, 0.4296,
1.31, 0.4240,
1.32, 0.4184,
1.33, 0.4129,
1.34, 0.4075,
1.35, 0.4020,
1.36, 0.3966,
1.37, 0.3912,
1.38, 0.3859,
1.39, 0.3806,
1.40, 0.3753,
1.41, 0.3701,
1.42, 0.3649,
1.43, 0.3597,
1.44, 0.3546,
1.45, 0.3495,
1.46, 0.3445,
1.47, 0.3394,
1.48, 0.3345,
1.49, 0.3295,
1.50, 0.3247,
1.51, 0.3198,
1.52, 0.3150,
1.53, 0.3102,
1.54, 0.3055,
1.55, 0.3008,
1.56, 0.2962,
1.57, 0.2916,
1.58, 0.2870,
1.59, 0.2825,
1.60, 0.2780,
1.61, 0.2736,
1.62, 0.2692,
1.63, 0.2649,
1.64, 0.2606,
1.65, 0.2563,
1.66, 0.2521,
1.67, 0.2480,
1.68, 0.2439,
1.69, 0.2398,
1.70, 0.2357,
1.71, 0.2318,
1.72, 0.2278,
1.73, 0.2239,
1.74, 0.2201,
1.75, 0.2163,
1.76, 0.2125,
1.77, 0.2088,
1.78, 0.2051,
1.79, 0.2015,
1.80, 0.1979,
1.81, 0.1944,
1.82, 0.1909,
1.83, 0.1874,
1.84, 0.1840,
1.85, 0.1806,
1.86, 0.1773,
1.87, 0.1740,
1.88, 0.1708,
1.89, 0.1676,
1.90, 0.1645,
1.91, 0.1614,
1.92, 0.1583,
1.93, 0.1553,
1.94, 0.1523,
1.95, 0.1494,
1.96, 0.1465,
1.97, 0.1436,
1.98, 0.1408,
1.99, 0.1381,
2.00, 0.1353,
2.01, 0.1326,
2.02, 0.1300,
2.03, 0.1274,
2.04, 0.1248,
2.05, 0.1223,
2.06, 0.1198,
2.07, 0.1174,
2.08, 0.1150,
2.09, 0.1126,
2.10, 0.1103,
2.11, 0.1080,
2.12, 0.1057,
2.13, 0.1035,
2.14, 0.1013,
2.15, 0.0991,
2.16, 0.0970,
2.17, 0.0949,
2.18, 0.0929,
2.19, 0.0909,
2.20, 0.0889,
2.21, 0.0870,
2.22, 0.0851,
2.23, 0.0832,
2.24, 0.0814,
2.25, 0.0796,
2.26, 0.0778,
2.27, 0.0760,
2.28, 0.0743,
2.29, 0.0727,
2.30, 0.0710,
2.31, 0.0694,
2.32, 0.0678,
2.33, 0.0662,
2.34, 0.0647,
2.35, 0.0632,
2.36, 0.0617,
2.37, 0.0603,
2.38, 0.0589,
2.39, 0.0575,
2.40, 0.0561,
2.41, 0.0548,
2.42, 0.0535,
2.43, 0.0522,
2.44, 0.0510,
2.45, 0.0497,
2.46, 0.0485,
2.47, 0.0473,
2.48, 0.0462,
2.49, 0.0450,
2.50, 0.0439,
2.51, 0.0428,
2.52, 0.0418,
2.53, 0.0407,
2.54, 0.0397,
2.55, 0.0387,
2.56, 0.0377,
2.57, 0.0368,
2.58, 0.0359,
2.59, 0.0349,
2.60, 0.0340,
2.61, 0.0332,
2.62, 0.0323,
2.63, 0.0315,
2.64, 0.0307,
2.65, 0.0299,
2.66, 0.0291,
2.67, 0.0283,
2.68, 0.0276,
2.69, 0.0268,
2.70, 0.0261,
2.71, 0.0254,
2.72, 0.0247,
2.73, 0.0241,
2.74, 0.0234,
2.75, 0.0228,
2.76, 0.0222,
2.77, 0.0216,
2.78, 0.0210,
2.79, 0.0204,
2.80, 0.0198,
2.81, 0.0193,
2.82, 0.0188,
2.83, 0.0182,
2.84, 0.0177,
2.85, 0.0172,
2.86, 0.0167,
2.87, 0.0163,
2.88, 0.0158,
2.89, 0.0154,
2.90, 0.0149,
2.91, 0.0145,
2.92, 0.0141,
2.93, 0.0137,
2.94, 0.0133,
2.95, 0.0129,
2.96, 0.0125,
2.97, 0.0121,
2.98, 0.0118,
2.99, 0.0114,
3.00, 0.0111,
};

/* matrix that contains the alpha and p parameters Range = f(E, alpha, p)
    First line is proton, second line is He... */

extern const double particle_parameters[][2] = {
    0.0022, 1.77,   //P
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
