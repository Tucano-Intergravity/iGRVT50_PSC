/**
 * @file tc_func.c
 * @author Heesung Shin (shs777@danam.co.kr)
 * @brief
 * @version 1.0.0
 * @date 2025-12-18
 *
 * @copyright Danam Systems Copyright (c) 2025
 */

/*==============================================================================
 * Include Files
 *============================================================================*/
/* --- includes --- */
 #include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <math.h>

/* --- FreeRTOS includes --- */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* --- User includes --- */
#include "sam_ctl.h"

/*==============================================================================
 * Gloabal Variables
 *============================================================================*/

/*==============================================================================
 * Gloabal Function
 *============================================================================*/
void ADS1263_Init(void);            // ADS1263 디바이스 초기화 함수
float ADS1263_GetTemperature( UInt8 ucCh );
float ADS1263_GetTemperatureTask( UInt8 ucCh );

/*==============================================================================
 * Local Variables
 *============================================================================*/

/*******************************************************************************************//**
 *
 *  @brief Type K Thermocouple Thermoelectric Voltage in mV from ITS-90
 *
 *      Table starts at low temp with each entry in 1??C increments
 */
const float typeK[] = {
/*                -9       -8       -7       -6       -5       -4       -3       -2       -1        0  */
/* -270 ??C */                                                                                   -6.458,
/* -260 ??C */  -6.457,  -6.456,  -6.455,  -6.453,  -6.452,  -6.450,  -6.448,  -6.446,  -6.444,  -6.441,
/* -250 ??C */  -6.438,  -6.435,  -6.432,  -6.429,  -6.425,  -6.421,  -6.417,  -6.413,  -6.408,  -6.404,
/* -240 ??C */  -6.399,  -6.393,  -6.388,  -6.382,  -6.377,  -6.370,  -6.364,  -6.358,  -6.351,  -6.344,
/* -230 ??C */  -6.337,  -6.329,  -6.322,  -6.314,  -6.306,  -6.297,  -6.289,  -6.280,  -6.271,  -6.262,
/* -220 ??C */  -6.252,  -6.243,  -6.233,  -6.223,  -6.213,  -6.202,  -6.192,  -6.181,  -6.170,  -6.158,
/* -210 ??C */  -6.147,  -6.135,  -6.123,  -6.111,  -6.099,  -6.087,  -6.074,  -6.061,  -6.048,  -6.035,
/* -200 ??C */  -6.021,  -6.007,  -5.994,  -5.980,  -5.965,  -5.951,  -5.936,  -5.922,  -5.907,  -5.891,
/* -190 ??C */  -5.876,  -5.861,  -5.845,  -5.829,  -5.813,  -5.797,  -5.780,  -5.763,  -5.747,  -5.730,
/* -180 ??C */  -5.713,  -5.695,  -5.678,  -5.660,  -5.642,  -5.624,  -5.606,  -5.588,  -5.569,  -5.550,
/* -170 ??C */  -5.531,  -5.512,  -5.493,  -5.474,  -5.454,  -5.435,  -5.415,  -5.395,  -5.374,  -5.354,
/* -160 ??C */  -5.333,  -5.313,  -5.292,  -5.271,  -5.250,  -5.228,  -5.207,  -5.185,  -5.163,  -5.141,
/* -150 ??C */  -5.119,  -5.097,  -5.074,  -5.052,  -5.029,  -5.006,  -4.983,  -4.960,  -4.936,  -4.913,
/* -140 ??C */  -4.889,  -4.865,  -4.841,  -4.817,  -4.793,  -4.768,  -4.744,  -4.719,  -4.694,  -4.669,
/* -130 ??C */  -4.644,  -4.618,  -4.593,  -4.567,  -4.542,  -4.516,  -4.490,  -4.463,  -4.437,  -4.411,
/* -120 ??C */  -4.384,  -4.357,  -4.330,  -4.303,  -4.276,  -4.249,  -4.221,  -4.194,  -4.166,  -4.138,
/* -110 ??C */  -4.110,  -4.082,  -4.054,  -4.025,  -3.997,  -3.968,  -3.939,  -3.911,  -3.882,  -3.852,
/* -100 ??C */  -3.823,  -3.794,  -3.764,  -3.734,  -3.705,  -3.675,  -3.645,  -3.614,  -3.584,  -3.554,
/*  -90 ??C */  -3.523,  -3.492,  -3.462,  -3.431,  -3.400,  -3.368,  -3.337,  -3.306,  -3.274,  -3.243,
/*  -80 ??C */  -3.211,  -3.179,  -3.147,  -3.115,  -3.083,  -3.050,  -3.018,  -2.986,  -2.953,  -2.920,
/*  -70 ??C */  -2.887,  -2.854,  -2.821,  -2.788,  -2.755,  -2.721,  -2.688,  -2.654,  -2.620,  -2.587,
/*  -60 ??C */  -2.553,  -2.519,  -2.485,  -2.450,  -2.416,  -2.382,  -2.347,  -2.312,  -2.278,  -2.243,
/*  -50 ??C */  -2.208,  -2.173,  -2.138,  -2.103,  -2.067,  -2.032,  -1.996,  -1.961,  -1.925,  -1.889,
/*  -40 ??C */  -1.854,  -1.818,  -1.782,  -1.745,  -1.709,  -1.673,  -1.637,  -1.600,  -1.564,  -1.527,
/*  -30 ??C */  -1.490,  -1.453,  -1.417,  -1.380,  -1.343,  -1.305,  -1.268,  -1.231,  -1.194,  -1.156,
/*  -20 ??C */  -1.119,  -1.081,  -1.043,  -1.006,  -0.968,  -0.930,  -0.892,  -0.854,  -0.816,  -0.778,
/*  -10 ??C */  -0.739,  -0.701,  -0.663,  -0.624,  -0.586,  -0.547,  -0.508,  -0.470,  -0.431,  -0.392,
/*    0 ??C */  -0.353,  -0.314,  -0.275,  -0.236,  -0.197,  -0.157,  -0.118,  -0.079,  -0.039,
/*                 0        1        2        3        4        5        6        7        8        9  */
/*    0 ??C */   0.000,   0.039,   0.079,   0.119,   0.158,   0.198,   0.238,   0.277,   0.317,   0.357,
/*   10 ??C */   0.397,   0.437,   0.477,   0.517,   0.557,   0.597,   0.637,   0.677,   0.718,   0.758,
/*   20 ??C */   0.798,   0.838,   0.879,   0.919,   0.960,   1.000,   1.041,   1.081,   1.122,   1.163,
/*   30 ??C */   1.203,   1.244,   1.285,   1.326,   1.366,   1.407,   1.448,   1.489,   1.530,   1.571,
/*   40 ??C */   1.612,   1.653,   1.694,   1.735,   1.776,   1.817,   1.858,   1.899,   1.941,   1.982,
/*   50 ??C */   2.023,   2.064,   2.106,   2.147,   2.188,   2.230,   2.271,   2.312,   2.354,   2.395,
/*   60 ??C */   2.436,   2.478,   2.519,   2.561,   2.602,   2.644,   2.685,   2.727,   2.768,   2.810,
/*   70 ??C */   2.851,   2.893,   2.934,   2.976,   3.017,   3.059,   3.100,   3.142,   3.184,   3.225,
/*   80 ??C */   3.267,   3.308,   3.350,   3.391,   3.433,   3.474,   3.516,   3.557,   3.599,   3.640,
/*   90 ??C */   3.682,   3.723,   3.765,   3.806,   3.848,   3.889,   3.931,   3.972,   4.013,   4.055,
/*  100 ??C */   4.096,   4.138,   4.179,   4.220,   4.262,   4.303,   4.344,   4.385,   4.427,   4.468,
/*  110 ??C */   4.509,   4.550,   4.591,   4.633,   4.674,   4.715,   4.756,   4.797,   4.838,   4.879,
/*  120 ??C */   4.920,   4.961,   5.002,   5.043,   5.084,   5.124,   5.165,   5.206,   5.247,   5.288,
/*  130 ??C */   5.328,   5.369,   5.410,   5.450,   5.491,   5.532,   5.572,   5.613,   5.653,   5.694,
/*  140 ??C */   5.735,   5.775,   5.815,   5.856,   5.896,   5.937,   5.977,   6.017,   6.058,   6.098,
/*  150 ??C */   6.138,   6.179,   6.219,   6.259,   6.299,   6.339,   6.380,   6.420,   6.460,   6.500,
/*  160 ??C */   6.540,   6.580,   6.620,   6.660,   6.701,   6.741,   6.781,   6.821,   6.861,   6.901,
/*  170 ??C */   6.941,   6.981,   7.021,   7.060,   7.100,   7.140,   7.180,   7.220,   7.260,   7.300,
/*  180 ??C */   7.340,   7.380,   7.420,   7.460,   7.500,   7.540,   7.579,   7.619,   7.659,   7.699,
/*  190 ??C */   7.739,   7.779,   7.819,   7.859,   7.899,   7.939,   7.979,   8.019,   8.059,   8.099,
/*  200 ??C */   8.138,   8.178,   8.218,   8.258,   8.298,   8.338,   8.378,   8.418,   8.458,   8.499,
/*  210 ??C */   8.539,   8.579,   8.619,   8.659,   8.699,   8.739,   8.779,   8.819,   8.860,   8.900,
/*  220 ??C */   8.940,   8.980,   9.020,   9.061,   9.101,   9.141,   9.181,   9.222,   9.262,   9.302,
/*  230 ??C */   9.343,   9.383,   9.423,   9.464,   9.504,   9.545,   9.585,   9.626,   9.666,   9.707,
/*  240 ??C */   9.747,   9.788,   9.828,   9.869,   9.909,   9.950,   9.991,  10.031,  10.072,  10.113,
/*  250 ??C */  10.153,  10.194,  10.235,  10.276,  10.316,  10.357,  10.398,  10.439,  10.480,  10.520,
/*  260 ??C */  10.561,  10.602,  10.643,  10.684,  10.725,  10.766,  10.807,  10.848,  10.889,  10.930,
/*  270 ??C */  10.971,  11.012,  11.053,  11.094,  11.135,  11.176,  11.217,  11.259,  11.300,  11.341,
/*  280 ??C */  11.382,  11.423,  11.465,  11.506,  11.547,  11.588,  11.630,  11.671,  11.712,  11.753,
/*  290 ??C */  11.795,  11.836,  11.877,  11.919,  11.960,  12.001,  12.043,  12.084,  12.126,  12.167,
/*  300 ??C */  12.209,  12.250,  12.291,  12.333,  12.374,  12.416,  12.457,  12.499,  12.540,  12.582,
/*  310 ??C */  12.624,  12.665,  12.707,  12.748,  12.790,  12.831,  12.873,  12.915,  12.956,  12.998,
/*  320 ??C */  13.040,  13.081,  13.123,  13.165,  13.206,  13.248,  13.290,  13.331,  13.373,  13.415,
/*  330 ??C */  13.457,  13.498,  13.540,  13.582,  13.624,  13.665,  13.707,  13.749,  13.791,  13.833,
/*  340 ??C */  13.874,  13.916,  13.958,  14.000,  14.042,  14.084,  14.126,  14.167,  14.209,  14.251,
/*  350 ??C */  14.293,  14.335,  14.377,  14.419,  14.461,  14.503,  14.545,  14.587,  14.629,  14.671,
/*  360 ??C */  14.713,  14.755,  14.797,  14.839,  14.881,  14.923,  14.965,  15.007,  15.049,  15.091,
/*  370 ??C */  15.133,  15.175,  15.217,  15.259,  15.301,  15.343,  15.385,  15.427,  15.469,  15.511,
/*  380 ??C */  15.554,  15.596,  15.638,  15.680,  15.722,  15.764,  15.806,  15.849,  15.891,  15.933,
/*  390 ??C */  15.975,  16.017,  16.059,  16.102,  16.144,  16.186,  16.228,  16.270,  16.313,  16.355,
/*  400 ??C */  16.397,  16.439,  16.482,  16.524,  16.566,  16.608,  16.651,  16.693,  16.735,  16.778,
/*  410 ??C */  16.820,  16.862,  16.904,  16.947,  16.989,  17.031,  17.074,  17.116,  17.158,  17.201,
/*  420 ??C */  17.243,  17.285,  17.328,  17.370,  17.413,  17.455,  17.497,  17.540,  17.582,  17.624,
/*  430 ??C */  17.667,  17.709,  17.752,  17.794,  17.837,  17.879,  17.921,  17.964,  18.006,  18.049,
/*  440 ??C */  18.091,  18.134,  18.176,  18.218,  18.261,  18.303,  18.346,  18.388,  18.431,  18.473,
/*  450 ??C */  18.516,  18.558,  18.601,  18.643,  18.686,  18.728,  18.771,  18.813,  18.856,  18.898,
/*  460 ??C */  18.941,  18.983,  19.026,  19.068,  19.111,  19.154,  19.196,  19.239,  19.281,  19.324,
/*  470 ??C */  19.366,  19.409,  19.451,  19.494,  19.537,  19.579,  19.622,  19.664,  19.707,  19.750,
/*  480 ??C */  19.792,  19.835,  19.877,  19.920,  19.962,  20.005,  20.048,  20.090,  20.133,  20.175,
/*  490 ??C */  20.218,  20.261,  20.303,  20.346,  20.389,  20.431,  20.474,  20.516,  20.559,  20.602,
/*  500 ??C */  20.644,  20.687,  20.730,  20.772,  20.815,  20.857,  20.900,  20.943,  20.985,  21.028,
/*  510 ??C */  21.071,  21.113,  21.156,  21.199,  21.241,  21.284,  21.326,  21.369,  21.412,  21.454,
/*  520 ??C */  21.497,  21.540,  21.582,  21.625,  21.668,  21.710,  21.753,  21.796,  21.838,  21.881,
/*  530 ??C */  21.924,  21.966,  22.009,  22.052,  22.094,  22.137,  22.179,  22.222,  22.265,  22.307,
/*  540 ??C */  22.350,  22.393,  22.435,  22.478,  22.521,  22.563,  22.606,  22.649,  22.691,  22.734,
/*  550 ??C */  22.776,  22.819,  22.862,  22.904,  22.947,  22.990,  23.032,  23.075,  23.117,  23.160,
/*  560 ??C */  23.203,  23.245,  23.288,  23.331,  23.373,  23.416,  23.458,  23.501,  23.544,  23.586,
/*  570 ??C */  23.629,  23.671,  23.714,  23.757,  23.799,  23.842,  23.884,  23.927,  23.970,  24.012,
/*  580 ??C */  24.055,  24.097,  24.140,  24.182,  24.225,  24.267,  24.310,  24.353,  24.395,  24.438,
/*  590 ??C */  24.480,  24.523,  24.565,  24.608,  24.650,  24.693,  24.735,  24.778,  24.820,  24.863,
/*  600 ??C */  24.905,  24.948,  24.990,  25.033,  25.075,  25.118,  25.160,  25.203,  25.245,  25.288,
/*  610 ??C */  25.330,  25.373,  25.415,  25.458,  25.500,  25.543,  25.585,  25.627,  25.670,  25.712,
/*  620 ??C */  25.755,  25.797,  25.840,  25.882,  25.924,  25.967,  26.009,  26.052,  26.094,  26.136,
/*  630 ??C */  26.179,  26.221,  26.263,  26.306,  26.348,  26.390,  26.433,  26.475,  26.517,  26.560,
/*  640 ??C */  26.602,  26.644,  26.687,  26.729,  26.771,  26.814,  26.856,  26.898,  26.940,  26.983,
/*  650 ??C */  27.025,  27.067,  27.109,  27.152,  27.194,  27.236,  27.278,  27.320,  27.363,  27.405,
/*  660 ??C */  27.447,  27.489,  27.531,  27.574,  27.616,  27.658,  27.700,  27.742,  27.784,  27.826,
/*  670 ??C */  27.869,  27.911,  27.953,  27.995,  28.037,  28.079,  28.121,  28.163,  28.205,  28.247,
/*  680 ??C */  28.289,  28.332,  28.374,  28.416,  28.458,  28.500,  28.542,  28.584,  28.626,  28.668,
/*  690 ??C */  28.710,  28.752,  28.794,  28.835,  28.877,  28.919,  28.961,  29.003,  29.045,  29.087,
/*  700 ??C */  29.129,  29.171,  29.213,  29.255,  29.297,  29.338,  29.380,  29.422,  29.464,  29.506,
/*  710 ??C */  29.548,  29.589,  29.631,  29.673,  29.715,  29.757,  29.798,  29.840,  29.882,  29.924,
/*  720 ??C */  29.965,  30.007,  30.049,  30.090,  30.132,  30.174,  30.216,  30.257,  30.299,  30.341,
/*  730 ??C */  30.382,  30.424,  30.466,  30.507,  30.549,  30.590,  30.632,  30.674,  30.715,  30.757,
/*  740 ??C */  30.798,  30.840,  30.881,  30.923,  30.964,  31.006,  31.047,  31.089,  31.130,  31.172,
/*  750 ??C */  31.213,  31.255,  31.296,  31.338,  31.379,  31.421,  31.462,  31.504,  31.545,  31.586,
/*  760 ??C */  31.628,  31.669,  31.710,  31.752,  31.793,  31.834,  31.876,  31.917,  31.958,  32.000,
/*  770 ??C */  32.041,  32.082,  32.124,  32.165,  32.206,  32.247,  32.289,  32.330,  32.371,  32.412,
/*  780 ??C */  32.453,  32.495,  32.536,  32.577,  32.618,  32.659,  32.700,  32.742,  32.783,  32.824,
/*  790 ??C */  32.865,  32.906,  32.947,  32.988,  33.029,  33.070,  33.111,  33.152,  33.193,  33.234,
/*  800 ??C */  33.275,  33.316,  33.357,  33.398,  33.439,  33.480,  33.521,  33.562,  33.603,  33.644,
/*  810 ??C */  33.685,  33.726,  33.767,  33.808,  33.848,  33.889,  33.930,  33.971,  34.012,  34.053,
/*  820 ??C */  34.093,  34.134,  34.175,  34.216,  34.257,  34.297,  34.338,  34.379,  34.420,  34.460,
/*  830 ??C */  34.501,  34.542,  34.582,  34.623,  34.664,  34.704,  34.745,  34.786,  34.826,  34.867,
/*  840 ??C */  34.908,  34.948,  34.989,  35.029,  35.070,  35.110,  35.151,  35.192,  35.232,  35.273,
/*  850 ??C */  35.313,  35.354,  35.394,  35.435,  35.475,  35.516,  35.556,  35.596,  35.637,  35.677,
/*  860 ??C */  35.718,  35.758,  35.798,  35.839,  35.879,  35.920,  35.960,  36.000,  36.041,  36.081,
/*  870 ??C */  36.121,  36.162,  36.202,  36.242,  36.282,  36.323,  36.363,  36.403,  36.443,  36.484,
/*  880 ??C */  36.524,  36.564,  36.604,  36.644,  36.685,  36.725,  36.765,  36.805,  36.845,  36.885,
/*  890 ??C */  36.925,  36.965,  37.006,  37.046,  37.086,  37.126,  37.166,  37.206,  37.246,  37.286,
/*  900 ??C */  37.326,  37.366,  37.406,  37.446,  37.486,  37.526,  37.566,  37.606,  37.646,  37.686,
/*  910 ??C */  37.725,  37.765,  37.805,  37.845,  37.885,  37.925,  37.965,  38.005,  38.044,  38.084,
/*  920 ??C */  38.124,  38.164,  38.204,  38.243,  38.283,  38.323,  38.363,  38.402,  38.442,  38.482,
/*  930 ??C */  38.522,  38.561,  38.601,  38.641,  38.680,  38.720,  38.760,  38.799,  38.839,  38.878,
/*  940 ??C */  38.918,  38.958,  38.997,  39.037,  39.076,  39.116,  39.155,  39.195,  39.235,  39.274,
/*  950 ??C */  39.314,  39.353,  39.393,  39.432,  39.471,  39.511,  39.550,  39.590,  39.629,  39.669,
/*  960 ??C */  39.708,  39.747,  39.787,  39.826,  39.866,  39.905,  39.944,  39.984,  40.023,  40.062,
/*  970 ??C */  40.101,  40.141,  40.180,  40.219,  40.259,  40.298,  40.337,  40.376,  40.415,  40.455,
/*  980 ??C */  40.494,  40.533,  40.572,  40.611,  40.651,  40.690,  40.729,  40.768,  40.807,  40.846,
/*  990 ??C */  40.885,  40.924,  40.963,  41.002,  41.042,  41.081,  41.120,  41.159,  41.198,  41.237,
/* 1000 ??C */  41.276,  41.315,  41.354,  41.393,  41.431,  41.470,  41.509,  41.548,  41.587,  41.626,
/* 1010 ??C */  41.665,  41.704,  41.743,  41.781,  41.820,  41.859,  41.898,  41.937,  41.976,  42.014,
/* 1020 ??C */  42.053,  42.092,  42.131,  42.169,  42.208,  42.247,  42.286,  42.324,  42.363,  42.402,
/* 1030 ??C */  42.440,  42.479,  42.518,  42.556,  42.595,  42.633,  42.672,  42.711,  42.749,  42.788,
/* 1040 ??C */  42.826,  42.865,  42.903,  42.942,  42.980,  43.019,  43.057,  43.096,  43.134,  43.173,
/* 1050 ??C */  43.211,  43.250,  43.288,  43.327,  43.365,  43.403,  43.442,  43.480,  43.518,  43.557,
/* 1060 ??C */  43.595,  43.633,  43.672,  43.710,  43.748,  43.787,  43.825,  43.863,  43.901,  43.940,
/* 1070 ??C */  43.978,  44.016,  44.054,  44.092,  44.130,  44.169,  44.207,  44.245,  44.283,  44.321,
/* 1080 ??C */  44.359,  44.397,  44.435,  44.473,  44.512,  44.550,  44.588,  44.626,  44.664,  44.702,
/* 1090 ??C */  44.740,  44.778,  44.816,  44.853,  44.891,  44.929,  44.967,  45.005,  45.043,  45.081,
/* 1100 ??C */  45.119,  45.157,  45.194,  45.232,  45.270,  45.308,  45.346,  45.383,  45.421,  45.459,
/* 1110 ??C */  45.497,  45.534,  45.572,  45.610,  45.647,  45.685,  45.723,  45.760,  45.798,  45.836,
/* 1120 ??C */  45.873,  45.911,  45.948,  45.986,  46.024,  46.061,  46.099,  46.136,  46.174,  46.211,
/* 1130 ??C */  46.249,  46.286,  46.324,  46.361,  46.398,  46.436,  46.473,  46.511,  46.548,  46.585,
/* 1140 ??C */  46.623,  46.660,  46.697,  46.735,  46.772,  46.809,  46.847,  46.884,  46.921,  46.958,
/* 1150 ??C */  46.995,  47.033,  47.070,  47.107,  47.144,  47.181,  47.218,  47.256,  47.293,  47.330,
/* 1160 ??C */  47.367,  47.404,  47.441,  47.478,  47.515,  47.552,  47.589,  47.626,  47.663,  47.700,
/* 1170 ??C */  47.737,  47.774,  47.811,  47.848,  47.884,  47.921,  47.958,  47.995,  48.032,  48.069,
/* 1180 ??C */  48.105,  48.142,  48.179,  48.216,  48.252,  48.289,  48.326,  48.363,  48.399,  48.436,
/* 1190 ??C */  48.473,  48.509,  48.546,  48.582,  48.619,  48.656,  48.692,  48.729,  48.765,  48.802,
/* 1200 ??C */  48.838,  48.875,  48.911,  48.948,  48.984,  49.021,  49.057,  49.093,  49.130,  49.166,
/* 1210 ??C */  49.202,  49.239,  49.275,  49.311,  49.348,  49.384,  49.420,  49.456,  49.493,  49.529,
/* 1220 ??C */  49.565,  49.601,  49.637,  49.674,  49.710,  49.746,  49.782,  49.818,  49.854,  49.890,
/* 1230 ??C */  49.926,  49.962,  49.998,  50.034,  50.070,  50.106,  50.142,  50.178,  50.214,  50.250,
/* 1240 ??C */  50.286,  50.322,  50.358,  50.393,  50.429,  50.465,  50.501,  50.537,  50.572,  50.608,
/* 1250 ??C */  50.644,  50.680,  50.715,  50.751,  50.787,  50.822,  50.858,  50.894,  50.929,  50.965,
/* 1260 ??C */  51.000,  51.036,  51.071,  51.107,  51.142,  51.178,  51.213,  51.249,  51.284,  51.320,
/* 1270 ??C */  51.355,  51.391,  51.426,  51.461,  51.497,  51.532,  51.567,  51.603,  51.638,  51.673,
/* 1280 ??C */  51.708,  51.744,  51.779,  51.814,  51.849,  51.885,  51.920,  51.955,  51.990,  52.025,
/* 1290 ??C */  52.060,  52.095,  52.130,  52.165,  52.200,  52.235,  52.270,  52.305,  52.340,  52.375,
/* 1300 ??C */  52.410,  52.445,  52.480,  52.515,  52.550,  52.585,  52.620,  52.654,  52.689,  52.724,
/* 1310 ??C */  52.759,  52.794,  52.828,  52.863,  52.898,  52.932,  52.967,  53.002,  53.037,  53.071,
/* 1320 ??C */  53.106,  53.140,  53.175,  53.210,  53.244,  53.279,  53.313,  53.348,  53.382,  53.417,
/* 1330 ??C */  53.451,  53.486,  53.520,  53.555,  53.589,  53.623,  53.658,  53.692,  53.727,  53.761,
/* 1340 ??C */  53.795,  53.830,  53.864,  53.898,  53.932,  53.967,  54.001,  54.035,  54.069,  54.104,
/* 1350 ??C */  54.138,  54.172,  54.206,  54.240,  54.274,  54.308,  54.343,  54.377,  54.411,  54.445,
/* 1360 ??C */  54.479,  54.513,  54.547,  54.581,  54.615,  54.649,  54.683,  54.717,  54.751,  54.785,
/* 1370 ??C */  54.819,  54.852,  54.886
};

/*==============================================================================
 * Local Function
 *============================================================================*/
/* ADS1263 Device Control */
 static void ADS1263_SPI_Transfer(uint8_t *tx, uint8_t *rx, size_t len);
static uint8_t ADS1263_ReadReg( uint8_t regAddress );
static void ADS1263_WriteReg( uint8_t regAddress, uint8_t data );
static void ADS1263_StartAdc( void );
static void ADS1263_StopAdc( void );
static int32_t ADS1263_ReadAdc( void );
static void ADS1263_SetChannel(uint8_t ainp, uint8_t ainm);
typedef void (*ADS1263_DelayFunc)( UInt32 delayMs );
static void ADS1263_BusyDelayMs( UInt32 delayMs );
static void ADS1263_TaskDelayMs( UInt32 delayMs );
static float ADS1263_GetTemperatureWithDelay( UInt8 ucCh, ADS1263_DelayFunc delayFunc );

static float ADC_CodeToVoltage(int32_t code);
static float voltage_to_ntc_resistance( float v_ntc );
static float ntc_resistance_to_temperature( float r_ntc );
static float adc_to_ntc_temperature( int32_t code );
static float typeK_mv_to_temp( float mv );
static float typeK_temp_to_mv(float temp_c);
static float adc_to_tc_temp( int32_t code, float cjTemp );
static float Thermocouple_Temp2mV( thermoTableCoefficientSet *tcSet, float temp);
static int Binary_Search(const float *table, float value, int min, int max);
static float Thermocouple_mV2Temp( thermoTableCoefficientSet *tcSet, float mV);
static float adc_to_tc_temp2( int32_t code, float cjTemp );



/*==============================================================================
 * Functions
 *============================================================================*/
thermoTableCoefficientSet typeK_mV2T = {
    .type = K,
    .min = -270.00,
    .max = 1372.00,
    .size = sizeof(typeK)/sizeof(typeK[0]),
    .table = typeK,
};

/* --- ADS1263 = SPI1 (폴링 8bit) + 수동 GPIO CS ---
 * 신규 HW: TC ADC가 SPI1(PC24 SCK/PC26 MISO/PC27 MOSI)에 연결,
 * CS는 수동 GPIO: #1 TCADC_SPICS=PC25, #2 TCADC2_SPICS=PD28 */
/* [핀 재확정] 사용자 도통 + atdf 핀번호 기준 실배선:
 *   SPICLK=pin130=PC24, SPICS=pin133=PC25, SPIMISO=pin13=PC26, SPIMOSI=pin12=PC27.
 *   (= SPI1 HW 페리페럴 핀과 동일. 이전 'PC25~28' 매핑은 오판이었음)
 *   비트뱅(Mode1 CPOL0/CPHA1, MSB first)로 해당 GPIO 구동.
 * 핀: PC24=SCLK(out), PC25=CS1(out), PC26=MISO(in), PC27=MOSI(out), PD28=CS2(out)
 * !! PC28=PG_P3V3D(전원굿) -> 절대 구동 금지(입력 유지). 이전 버전이 잘못 구동해 3.3V 불안정 유발. */
/* [확정] 사용자 도통측정 + Harmony 일치: SCLK=PC24(p130) CS=PC25(p133) MISO=PC26 MOSI=PC27 */
#define TADS_SCK   (24U)   /* PC24 = TCADC_SPICLK  (MCU핀130) */
#define TADS_CS1   (25U)   /* PC25 = TCADC_SPICS   (MCU핀133, R116) */
#define TADS_MISO  (26U)   /* PC26 = TCADC_SPIMISO (ADS pin13 DOUT) */
#define TADS_MOSI  (27U)   /* PC27 = TCADC_SPIMOSI (ADS pin12 DIN) */
#define TCK_HI()    (PIOC_REGS->PIO_SODR = (1UL<<TADS_SCK))
#define TCK_LO()    (PIOC_REGS->PIO_CODR = (1UL<<TADS_SCK))
#define TMOSI_HI()  (PIOC_REGS->PIO_SODR = (1UL<<TADS_MOSI))
#define TMOSI_LO()  (PIOC_REGS->PIO_CODR = (1UL<<TADS_MOSI))
#define TMISO_RD()  ((PIOC_REGS->PIO_PDSR >> TADS_MISO) & 1UL)

static inline void tc_delay(void){ volatile uint32_t d; for(d=0; d<40U; d++){ __asm__ volatile("nop"); } }

/* [TC SPI 모드] 실측 확정: Mode1(CPHA1)이 정답 (ID=0x23 정상응답). Mode0는 0x11 쓰레기.
 *  1 = Mode1 (기본/정답): 상승엣지에 MOSI 변경, 하강 뒤 DOUT 샘플.
 *  0 = Mode0: DIN을 SCLK LOW에서 세팅, 상승엣지 DOUT 샘플 (이 칩엔 안 맞음).
 * tcmode 명령으로 런타임 토글. (아까 0xFF는 모드 아니라 하드웨어였고 전원사이클로 해결됨) */
static uint8_t s_tcSpiMode = 1U;
void  ADS1263_SetSpiMode( UInt8 m ) { s_tcSpiMode = m ? 1U : 0U; }
UInt8 ADS1263_GetSpiMode( void )    { return s_tcSpiMode; }

static void SPI1_TC_Init(void)
{
    /* SPI1 HW 비활성화 (이 배선으론 못 씀) */
    SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;

    /* PC25/26/27/28 을 PIO(GPIO)로 회수 */
    PIOC_REGS->PIO_PER  = (1UL<<TADS_SCK)|(1UL<<TADS_CS1)|(1UL<<TADS_MISO)|(1UL<<TADS_MOSI);
    PIOC_REGS->PIO_PUDR = (1UL<<TADS_SCK)|(1UL<<TADS_CS1)|(1UL<<TADS_MOSI);   /* 출력핀 풀 제거 */
    /* [진단] MISO(PC27)에 풀업: DOUT 미연결(핀12 냉납)이면 0xFF로 읽혀 단선 확인됨 */
    PIOC_REGS->PIO_PUER = (1UL<<TADS_MISO);
    /* 방향: SCK/CS1/MOSI=output, MISO=input */
    PIOC_REGS->PIO_OER  = (1UL<<TADS_SCK)|(1UL<<TADS_CS1)|(1UL<<TADS_MOSI);
    PIOC_REGS->PIO_ODR  = (1UL<<TADS_MISO);
    /* idle: SCK low(CPOL0), MOSI low, CS1 high(deselect) */
    PIOC_REGS->PIO_CODR = (1UL<<TADS_SCK)|(1UL<<TADS_MOSI);
    PIOC_REGS->PIO_SODR = (1UL<<TADS_CS1);

    /* PC28은 ADS SPI에 미사용 -> 안전하게 입력(high-Z)으로 해제 (구동 금지) */
    PIOC_REGS->PIO_PER  = (1UL<<28);
    PIOC_REGS->PIO_ODR  = (1UL<<28);
    PIOC_REGS->PIO_PUDR = (1UL<<28);

    /* CS#2 = PD28 (정상매핑) idle high */
    TCADC2_SPICS_OutputEnable();  TCADC2_SPICS_Set();
}

/* MSB first. 모드는 s_tcSpiMode로 선택 (Mode0=정석, Mode1=이전). tcmode로 토글. */
static uint8_t SPI1_TC_Byte(uint8_t out)
{
    uint8_t in = 0U;
    int8_t  i;
    for( i = 7; i >= 0; i-- )
    {
        if( s_tcSpiMode == 0U )
        {
            /* Mode0(CPHA0): DIN을 SCLK LOW에서 안정 → 상승엣지에 ADS가 DIN 래치 & 호스트가 DOUT 샘플 → 하강에 ADS가 DOUT 갱신 */
            if( (out >> i) & 1U ) { TMOSI_HI(); } else { TMOSI_LO(); }
            tc_delay();
            TCK_HI();
            if( TMISO_RD() ) { in |= (uint8_t)(1U << i); }
            tc_delay();
            TCK_LO();
        }
        else
        {
            /* Mode1(CPHA1, 이전): 상승 뒤 MOSI 세팅, 하강 뒤 DOUT 샘플 */
            TCK_HI();
            if( (out >> i) & 1U ) { TMOSI_HI(); } else { TMOSI_LO(); }
            tc_delay();
            TCK_LO();
            tc_delay();
            if( TMISO_RD() ) { in |= (uint8_t)(1U << i); }
        }
    }
    return in;
}

/* 현재 선택된 ADS1263 디바이스 (1 또는 2) — CS 분기에 사용 */
static uint8_t s_tcDev = 1;
void ADS1263_SetDevice( UInt8 dev ) { s_tcDev = (dev == 2) ? 2 : 1; }

/* [TC 보정] VBIAS(레벨시프트)/PGA bypass 토글 — floating 열전대 공통모드 진단·해결용.
 * 데이터시트 §10.1.1: 절연(floating) TC는 공통모드를 입력범위 내로 잡아야 함.
 *  - VBIAS = POWER.bit1 -> AINCOM에 (AVDD+AVSS)/2 = 2.5V 인가(레벨시프트).
 *  - bypass= MODE2.bit7 -> PGA 우회(gain1, 입력 rail-to-rail). */
static uint8_t s_tcVbias  = 1U;   /* 1=VBIAS on (기본: floating TC 권장) */
static uint8_t s_tcBypass = 1U;   /* [확정] 1=PGA bypass(gain1,rail-to-rail) 기본
                                   *  floating TC 공통모드가 AVSS 레일 근처라 PGA(gain32)는 offset 큼.
                                   *  bypass는 입력 rail-to-rail이라 정상 차동 측정됨 (-19mV->-2.9mV 검증) */

/* [TC 0점] 채널별 잔류 offset(mV). 쇼트 입력에서 tczero로 캡처 -> 측정값에서 뺌. */
static float   s_tcOffmV[2][4] = { {0,0,0,0}, {0,0,0,0} };   /* ADS#1=4ch(SEN1~4), ADS#2=2ch(SEN5~6) */
static float   s_curTcOffmV    = 0.0f;   /* GetTemperature가 변환 직전 설정 */

/* [진단] 채널별 마지막 raw ADC code (CH0~3=TC, 4=CJ). 0xFFFFFFFF=SPI 무응답(칩 DOUT high-Z),
 * 0x00000000=리드불가. 정상 변환이면 의미있는 32bit 값. 운영 TcPrint에서 hex로 노출. */
static int32_t s_tcRawCode[2][5] = { {0,0,0,0,0}, {0,0,0,0,0} };
int32_t ADS1263_GetRawCode( UInt8 dev, UInt8 ch ) { return s_tcRawCode[(dev==2)?1:0][(ch>4U)?4U:ch]; }
static uint8_t tc_chidx( uint8_t ainp ) { uint8_t c=(uint8_t)(ainp/2U); return (c>3U)?3U:c; }
void  ADS1263_SetTcOffsetCh( UInt8 dev, uint8_t ainp, float mv ) { s_tcOffmV[(dev==2)?1:0][tc_chidx(ainp)] = mv; }
float ADS1263_GetTcOffsetCh( UInt8 dev, uint8_t ainp ) { return s_tcOffmV[(dev==2)?1:0][tc_chidx(ainp)]; }
void  ADS1263_SetVbias ( UInt8 on ) { s_tcVbias  = on ? 1U : 0U; }
void  ADS1263_SetBypass( UInt8 on ) { s_tcBypass = on ? 1U : 0U; }
UInt8 ADS1263_GetVbias ( void ) { return s_tcVbias; }
UInt8 ADS1263_GetBypass( void ) { return s_tcBypass; }

/* TC 측정용 MODE2: bypass면 0x8A(우회,gain1,DR=0xA), 아니면 0x5A(gain32,DR=0xA) */
static uint8_t tc_mode2_val( void ) { return s_tcBypass ? 0x8AU : 0x5AU; }
/* 현재 디바이스 POWER에 VBIAS 반영 (INTREF on 유지) */
static void tc_apply_power( void ) { ADS1263_WriteReg( ADS1263_POWER, s_tcVbias ? 0x03U : 0x01U ); }

/* [디버그] 지정 디바이스의 레지스터 1개 읽기 (SPI 링크 HW/FW 판정용) */
/* [디버그] RREG(reg) 4바이트 raw 덤프 -> MCU가 받은 모든 바이트 확인 */
void ADS1263_DbgRaw4( UInt8 dev, UInt8 reg, UInt8 *rx4 )
{
    uint8_t tx[4];
    tx[0] = (uint8_t)(ADS1263_READ_ADD | reg);
    tx[1] = 0x00; tx[2] = 0x00; tx[3] = 0x00;
    ADS1263_SetDevice( dev );
    ADS1263_SPI_Transfer( tx, rx4, 4 );
    ADS1263_SetDevice( 1 );
}

UInt8 ADS1263_DbgReadReg( UInt8 dev, UInt8 reg )
{
    UInt8 v;
    ADS1263_SetDevice( dev );
    v = ADS1263_ReadReg( reg );
    ADS1263_SetDevice( 1 );
    return v;
}

/* [디버그] WREG -> RREG 왕복: 알려진 고정값을 레지스터에 쓰고 되읽음.
 * 되읽은 값 == 쓴 값 이면 SPI 양방향(MOSI/MISO) + ADS1263 IC 정상. */
UInt8 ADS1263_DbgWrRd( UInt8 dev, UInt8 reg, UInt8 val )
{
    UInt8 rb;
    ADS1263_SetDevice( dev );
    ADS1263_WriteReg( reg, val );
    rb = ADS1263_ReadReg( reg );
    ADS1263_SetDevice( 1 );
    return rb;
}

/* CS#1 = PC26(보드 실배선), CS#2 = PD28 */
static void ads_cs_low(void)  { if(s_tcDev == 2) TCADC2_SPICS_Clear(); else PIOC_REGS->PIO_CODR = (1UL<<TADS_CS1); }
static void ads_cs_high(void) { if(s_tcDev == 2) TCADC2_SPICS_Set();   else PIOC_REGS->PIO_SODR = (1UL<<TADS_CS1); }

static void ADS1263_SPI_Transfer(uint8_t *tx, uint8_t *rx, size_t len)
{
    size_t i;
    (void)(SPI1_REGS->SPI_RDR);            /* flush */
    ads_cs_low();                          /* 선택 디바이스 CS low */
    tc_delay();                            /* td(CSSC): CS low -> 첫 SCLK 최소 50ns 확보 */
    for( i = 0; i < len; i++ )
    {
        rx[i] = SPI1_TC_Byte(tx[i]);
    }
    ads_cs_high();                         /* deselect */
}

static uint8_t ADS1263_ReadReg( uint8_t regAddress )
{
    uint8_t data = 0;
    uint8_t readCmd[4] = {0};
    uint8_t rx[4] = {0};

    readCmd[0] = ADS1263_READ_ADD | regAddress;
    readCmd[1] = 0x00;        //according to datasheet (OPCODE2 byte for RREG Command for one register)
    readCmd[2] = 0x00;
    readCmd[3] = 0x00;

    ADS1263_SPI_Transfer(readCmd, rx, 4);

    data = rx[2];

    return data;
}

static void ADS1263_WriteReg( uint8_t regAddress, uint8_t data )
{
    uint8_t writeCmd[3] = {0};
    uint8_t rx[3] = {0};

    writeCmd[0] = ADS1263_WRITE_ADD | regAddress;
    writeCmd[1] = 0x00;                  //according to datasheet (OPCODE2 byte for WREG Command for one register)
    writeCmd[2] = data;
    
    ADS1263_SPI_Transfer(writeCmd, rx, 3);
}

static void ADS1263_StartAdc( void )
{
    uint8_t writeCmd[3] = {0};
    uint8_t rx[3] = {0};

    writeCmd[0] = ADS1263_START1_CMD;
    writeCmd[1] = 0x00;                  //according to datasheet (OPCODE2 byte for WREG Command for one register)
    writeCmd[2] = 0x00;

    ADS1263_SPI_Transfer(writeCmd, rx, 3);
}

static void ADS1263_StopAdc( void )
{
    uint8_t writeCmd[3] = {0};
    uint8_t rx[3] = {0};

    writeCmd[0] = ADS1263_STOP1_CMD;
    writeCmd[1] = 0x00;                  //according to datasheet (OPCODE2 byte for WREG Command for one register)
    writeCmd[2] = 0x00;

    ADS1263_SPI_Transfer(writeCmd, rx, 3);
}

/* [진단] START 후 DOUT/DRDY(공유핀13=MISO, CS low 중)가 "변환완료" 신호를 주는지 스캔.
 *  - 전용 DRDY(14)는 MCU 미연결 -> 공유 DOUT/DRDY = TCADC_SPIMISO(PC26)로만 판정 가능.
 *  - 데이터시트: 변환 완료시 DOUT/DRDY가 LOW로 떨어짐(CS low 일 때만 유효).
 *  - trans>0 = DRDY 토글 = 칩이 실제로 변환 동작중 / 계속 HIGH = 변환신호 없음
 *    (START 미반영/내부클럭 미동작) / 계속 LOW = stuck(디지털 이상). */
/* [진단] HW SPI1(peripheral C, NPCS1=PC25)로 dev1 ID 읽기 — 비트뱅 대신 하드웨어 SPI 교차검증.
 * 보드 핀(PC24=SPCK/PC25=NPCS1/PC26=MISO/PC27=MOSI)이 SPI1 peripheral C와 동일 → 네이티브 가능.
 * 결과: rx[2]=ID. 0x2X면 HW SPI로 칩 응답=비트뱅 문제였음. 0x00이면 칩 디지털 문제 확정. */
void ADS1263_HwIdTest( UInt8 *rx4 )
{
    const uint32_t M = (1UL<<24)|(1UL<<25)|(1UL<<26)|(1UL<<27);
    uint8_t tx[4];
    int i;
    tx[0] = (uint8_t)(ADS1263_READ_ADD | 0x00); tx[1]=0; tx[2]=0; tx[3]=0;

    /* PC24~27 -> peripheral C (ABCDSR2=1,ABCDSR1=0), PIO 해제 */
    PIOC_REGS->PIO_ABCDSR[0] &= ~M;
    PIOC_REGS->PIO_ABCDSR[1] |=  M;
    PIOC_REGS->PIO_PDR = M;

    /* SPI1 master, mode1, 8bit, NPCS1, CSAAT(4바이트 동안 CS 유지) */
    SPI1_REGS->SPI_CR  = SPI_CR_SPIDIS_Msk | SPI_CR_SWRST_Msk;
    SPI1_REGS->SPI_MR  = SPI_MR_MSTR_Msk | SPI_MR_PCS_NPCS1 | SPI_MR_MODFDIS_Msk;
    SPI1_REGS->SPI_CSR[1] = SPI_CSR_CPOL_IDLE_LOW | SPI_CSR_NCPHA_VALID_TRAILING_EDGE
                          | SPI_CSR_BITS_8_BIT | SPI_CSR_SCBR(150) | SPI_CSR_CSAAT_Msk;
    SPI1_REGS->SPI_CR  = SPI_CR_SPIEN_Msk;
    (void)(SPI1_REGS->SPI_RDR);

    for( i = 0; i < 4; i++ )
    {
        if( i == 3 ) { SPI1_REGS->SPI_CR = SPI_CR_LASTXFER_Msk; }
        while( (SPI1_REGS->SPI_SR & SPI_SR_TDRE_Msk) == 0U ){}
        SPI1_REGS->SPI_TDR = tx[i];
        while( (SPI1_REGS->SPI_SR & SPI_SR_RDRF_Msk) == 0U ){}
        rx4[i] = (uint8_t)(SPI1_REGS->SPI_RDR & 0xFFU);
    }
    SPI1_REGS->SPI_CR = SPI_CR_SPIDIS_Msk;

    /* 비트뱅 핀/상태 복원 */
    SPI1_TC_Init();
}

void ADS1263_DbgDrdyScan( UInt8 dev, int *pLows, int *pTrans )
{
    int i, lows = 0, trans = 0, prev = -1, lvl;
    ADS1263_SetDevice( dev );
    ADS1263_StartAdc();                    /* START1 (START핀은 init에서 LOW 유지) */
    ads_cs_low();                          /* DOUT/DRDY 관찰 위해 CS low 유지 */
    for( i = 0; i < 200; i++ )
    {
        lvl = (int)TMISO_RD();
        if( lvl == 0 ) { lows++; }
        if( (prev >= 0) && (lvl != prev) ) { trans++; }
        prev = lvl;
        SYSTICK_DelayMs( 2 );              /* 약 400ms 관찰 */
    }
    ads_cs_high();
    ADS1263_SetDevice( 1 );
    if( pLows  != NULL ) { *pLows  = lows; }
    if( pTrans != NULL ) { *pTrans = trans; }
}

static int32_t ADS1263_ReadAdc( void )
{
    int32_t i;
    int32_t msg = 0;
    uint8_t readCmd[7] = {0};
    uint8_t rx[7] = {0};

    readCmd[0] = ADS1263_RDATA1_CMD;
    readCmd[1] = 0x00;        //according to datasheet (OPCODE2 byte for RREG Command for one register)
    readCmd[2] = 0x00;
    readCmd[3] = 0x00;
    readCmd[4] = 0x00;
    readCmd[5] = 0x00;
    readCmd[6] = 0x00;

    ADS1263_SPI_Transfer(readCmd, rx, 7);

    msg |= rx[2] << 24;
    msg |= rx[3] << 16;
    msg |= rx[4] << 8;
    msg |= rx[5];

    return msg;
}

static void ADS1263_SetChannel(uint8_t ainp, uint8_t ainm)
{
    uint8_t value = ((ainp & 0x0F) << 4) | (ainm & 0x0F);
    ADS1263_WriteReg( ADS1263_INPMUX, value );  // INPMUX
}


static float ADC_CodeToVoltage(int32_t code)
{
    float tmp;
    uint8_t buffer = 0;
    uint8_t gain_exp;

    buffer = ADS1263_ReadReg( ADS1263_MODE2 );
    if( buffer & 0x80U ) { gain_exp = 0U; }                     /* PGA bypass -> gain 1 */
    else                 { gain_exp = (buffer & 0x70U) >> 4; }  /* GAIN bits[6:4] */

    tmp = ((float)code * VREF) / (powf( 2.0f, (float)gain_exp )*ADC_FS);
    return tmp;
}

/* [진단] TC 채널 raw 차동전압(mV) 읽기 — 열전대 검증용.
 * 쇼트접점=상온이면 ≈0mV, 접점 가열하면 상승(Type-K ≈ 41µV/°C).
 * gain=32(MODE2=0x5A)로 측정. dev=1/2, ainp/ainm = AIN 핀번호. */
float ADS1263_DbgReadmV( UInt8 dev, uint8_t ainp, uint8_t ainm )
{
    int32_t code;
    int64_t acc = 0;
    int     i;
    float   v;
    ADS1263_SetDevice( dev );
    tc_apply_power();                           /* VBIAS(floating TC 공통모드) 반영 */
    ADS1263_SetChannel( ainp, ainm );
    ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );   /* gain32 or bypass */
    ADS1263_StartAdc();
    SYSTICK_DelayMs( 50 );                       /* 채널/게인 전환 settle */
    for( i = 0; i < 8; i++ )                     /* 8회 평균(노이즈 저감) */
    {
        ADS1263_StartAdc();                      /* pulse 변환 1회 트리거 */
        SYSTICK_DelayMs( 10 );
        acc += (int64_t)ADS1263_ReadAdc();
    }
    code = (int32_t)( acc / 8 );
    v = ADC_CodeToVoltage( code );             /* volts */
    ADS1263_SetDevice( 1 );
    return v * 1000.0f;                          /* mV */
}

/* [CJ/진단] 지정 디바이스 내부 온도센서(다이온도) 읽기 -> 냉접점(CJ) 온도(°C) */
float ADS1263_ReadInternalTemp( UInt8 dev )
{
    int32_t code;
    float   v_uV;
    ADS1263_SetDevice( dev );
    ADS1263_SetChannel( 0x0B, 0x0B );           /* INPMUX=0xBB: 내부 온도센서 */
    ADS1263_WriteReg( ADS1263_MODE2, 0x0A );    /* gain1 */
    ADS1263_StartAdc();
    SYSTICK_DelayMs( 50 );
    code = ADS1263_ReadAdc();
    v_uV = ADC_CodeToVoltage( code ) * 1.0e6f;
    ADS1263_SetDevice( 1 );
    return 25.0f + ( v_uV - 122400.0f ) / 420.0f;   /* 122.4mV@25°C, 420µV/°C */
}

/* [진단] ADC1 self offset calibration (SFOCAL1) — 칩 자체 offset 보정(내부 입력 쇼트).
 * 보정 후 OFCAL 레지스터 자동 적용. 외부 공통모드/배선 offset은 보정 안 됨. */
void ADS1263_SelfOffsetCal( UInt8 dev )
{
    uint8_t cmd = ADS1263_SFOCAL1_CMD;
    uint8_t rx  = 0;
    ADS1263_SetDevice( dev );
    tc_apply_power();
    ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );   /* 운용 게인에서 보정 */
    ADS1263_StartAdc();
    SYSTICK_DelayMs( 30 );
    ADS1263_SPI_Transfer( &cmd, &rx, 1 );       /* SFOCAL1 단일 커맨드 */
    SYSTICK_DelayMs( 250 );                      /* 보정 완료 대기 */
    ADS1263_SetDevice( 1 );
}

static float voltage_to_ntc_resistance( float v_ntc )
{
    if (v_ntc <= 0.0f) return 1e9f;
    if (v_ntc >= (VREF - 1e-6f)) return 1.0f;

    return R_FIXED * v_ntc / (VREF - v_ntc);
}


static float ntc_resistance_to_temperature( float r_ntc )
{
    float inv_T =
        (1.0f / NTC_T0) +
        (1.0f / NTC_BETA) * logf(r_ntc / NTC_R0);

    float temp_K = 1.0f / inv_T;
    return temp_K - 273.15f;
}

static float adc_to_ntc_temperature( int32_t code )
{
    float v_ntc;
    float r_ntc;
    float temp_c;

    v_ntc = ADC_CodeToVoltage( code );
    
    if (v_ntc < 0.0f) v_ntc = -v_ntc;

    r_ntc = voltage_to_ntc_resistance(v_ntc);

    temp_c = ntc_resistance_to_temperature(r_ntc);

    return temp_c;
}

static float typeK_mv_to_temp( float mv )
{
    int i;
    float mv0, mv1;
    float t;
    
    for (i = 0; i < TYPEK_TABLE_SIZE - 1; i++)
    {
        mv0 = typeK[i];
        mv1 = typeK[i + 1];

        if (mv >= mv0 && mv <= mv1)
        {
            t = (float)i +
                (mv - mv0) / (mv1 - mv0);
            return t - 270.0f; // 테이블 시작 온도
        }
    }

    return NAN; // 범위 초과
}

static float typeK_temp_to_mv(float temp_c)
{
    int idx;
    float frac;
    float mv0, mv1;

    /* 1. 범위 체크 */
    if (temp_c < TYPEK_TABLE_MIN_TEMP || temp_c > TYPEK_TABLE_MAX_TEMP)
    {
        return NAN;
    }

    /* 2. 테이블 인덱스 계산 */
    idx = (int)floorf(temp_c - TYPEK_TABLE_MIN_TEMP);

    /* 정확히 정수 온도인 경우 */
    if ((idx >= TYPEK_TABLE_SIZE - 1) || (temp_c == (float)((int)temp_c)))
    {
        return typeK[idx];
    }

    /* 3. 소수점 보간 */
    frac = temp_c - floorf(temp_c);

    mv0 = typeK[idx];
    mv1 = typeK[idx + 1];

    return mv0 + frac * (mv1 - mv0);
}

static float adc_to_tc_temp( int32_t code, float cjTemp )
{
    float tc_vol;
    float cj_vol;
    float tot_vol;
    float temp;

    tc_vol = ADC_CodeToVoltage( code );
    cj_vol = typeK_temp_to_mv( cjTemp );
    tot_vol = (tc_vol*1000.0f)+cj_vol;
    printf( "%0.2lf = %0.2lf + %0.2lf\r\n", tot_vol, tc_vol, cj_vol);
    temp = typeK_mv_to_temp( tot_vol );

    return temp;
}

static float Thermocouple_Temp2mV( thermoTableCoefficientSet *tcSet, float temp)
{
	int tcIndex;

	// if previous math gave NAN, return NAN.
	if ( isnan(temp) ) return NAN;   /* [수정] float==NAN은 항상 false -> isnan() */

	tcIndex = ((int) temp) - tcSet->min;

	if ( tcIndex < 0 )
		return NAN;							// Temp below table min, return NAN
	else if ( tcIndex < 1 )
		return tcSet->min;					// Temp at min, return min
	else if ( tcIndex > (tcSet->max - tcSet->min) )
		return NAN;							// Temp above table max, return NAN
	else if ( tcIndex > (tcSet->max - tcSet->min - 1) )
		return tcSet->max;					// Temp at table max, return max

	// Linear Interpolation
	// mVx = m(tx - t0) + mV0; m = (mV1-mV0)/(temp1-temp0) = (mV1-mV0)/1
	// tx = mV0 + (tx - t0)*(mV1 - mV0)
	if ( (tcSet->table[tcIndex+1] - tcSet->table[tcIndex]) == 0 ) {  	// Both index are the same, choose upper one
		return( tcSet->table[tcIndex]  );
	} else {
		return( tcSet->table[tcIndex] + (temp - (tcSet->min + tcIndex)) * (tcSet->table[tcIndex+1] - tcSet->table[tcIndex]));
	}
}
static int Binary_Search(const float *table, float value, int min, int max)
{
	int midPt = (min + max)/2;

	if ( (midPt == max) || (midPt == min) )
		return( midPt );
	else {
	    if ( value < table[midPt])
	        return Binary_Search( table, value, min, midPt - 1 );
	    else
	        return Binary_Search(table, value, midPt + 1, max );

	}
}

static float Thermocouple_mV2Temp( thermoTableCoefficientSet *tcSet, float mV)
{
	int tcIndex;

	// if previous math gave NAN, return NAN.
	if ( isnan(mV) ) return NAN;   /* [수정] float==NAN은 항상 false -> isnan() */

	// test to see if the mv value is within range.
	// interpolation method requires one above and one below
	// the 'found' value.
	if ( mV < tcSet->table[0] )
		return NAN;
	else if ( mV <= tcSet->table[1] )
		return tcSet->min;
	else if ( mV > tcSet->table[tcSet->max - tcSet->min] )
		return NAN;
	if ( mV > tcSet->table[tcSet->max - tcSet->min - 1] )
		return tcSet->max;

	tcIndex = Binary_Search( tcSet->table, mV, 0, tcSet->size - 1 );

	// Linear Interpolation
	// tx = m(mVx - mV0) + t0; m = (t1-t0)/(mV1-mV0) = 1/(mV1-mV0)
	// tx = t0 + (mVx - mV0)/(mV1 - mV0)
	if ( (tcSet->table[tcIndex+1] - tcSet->table[tcIndex]) == 0 ) {  	// Both index are the same, choose upper one
		return( tcIndex + tcSet->min  );
	} else {
		return( tcIndex + tcSet->min + (mV - tcSet->table[tcIndex])/(tcSet->table[tcIndex+1] - tcSet->table[tcIndex]) );
	}
}

static float adc_to_tc_temp2( int32_t code, float cjTemp )
{
    float TCmV = 0.0f;
    float tc_vol = 0.0f;
    float CJmV = 0.0f;
    float mV = 0.0f;
    float temp = 0.0f;

    tc_vol = ADC_CodeToVoltage( code );
    TCmV = tc_vol*1000.0f - s_curTcOffmV;   /* 채널 0점 보정 적용 */

    thermoTableCoefficientSet *tcSet = NULL;
    tcSet  = &typeK_mV2T;

	// Compute Thermoelectric Voltage of Thermocouple's cold junction temperature
  	CJmV = Thermocouple_Temp2mV( tcSet, cjTemp );

  	// Total Thermoelectric Voltage
  	mV = TCmV + CJmV;

	tcSet  = &typeK_mV2T;

  	// Interpolate Temperature
  	temp = Thermocouple_mV2Temp( tcSet, mV );

  	return( temp );
}

void ADS1263_Init(void)
{
    /* SPI1 + 수동 CS 초기화 (신규 HW: TC ADC = SPI1) */
    SPI1_TC_Init();

    /* RESET (ADS1263 RESET_PWDN_N, active-low) : PA29 = TCADC_RESET
     * [수정] 기존 100ms LOW는 데이터시트상 9ms(65536 fCLK) 초과 = POWER-DOWN 진입(디지털 LDO off
     *  -> BYPASS가 DVDD로 뜸). 정상 RESET은 ≥4 fCLK ~ <9ms. 짧은 펄스로 파워다운 회피. */
    TCADC_RESET_Set();          /* 우선 해제 상태 보장 */
    SYSTICK_DelayMs(10);
    TCADC_RESET_Clear();        /* RESET low (정상 리셋) */
    SYSTICK_DelayMs(1);         /* ~1ms: >4 fCLK 리셋, <9ms 파워다운 회피 */
    TCADC_RESET_Set();          /* RESET 해제 */
    SYSTICK_DelayMs(50);        /* POR ready(9ms) + ref settle 여유 */

    /* START (ADS1263 START pin) : PC23 = TCADC_START */
    TCADC_START_Clear();
    SYSTICK_DelayMs(5);

    /* 디바이스 #1, #2 동일 설정 (RESET/START는 공유 신호) */
    UInt8 dev;
    for( dev = 1; dev <= 2; dev++ )
    {
        ADS1263_SetDevice( dev );
        /* POWER: INTREF on + VBIAS on (floating 열전대 공통모드 2.5V 레벨시프트) */
        ADS1263_WriteReg( ADS1263_POWER, s_tcVbias ? 0x03U : 0x01U );
        ADS1263_WriteReg( ADS1263_MODE0, ADS1263_MODE0_SETUP );   /* MODE0 */
        ADS1263_WriteReg( ADS1263_MODE1, ADS1263_MODE1_SINC4 );   /* MODE1 */
        ADS1263_WriteReg( ADS1263_MODE2, 0x0A );                  /* MODE2 GAIN/SPS */
    }
    ADS1263_SetDevice( 1 );     /* 기본 디바이스 #1 */
}

static void ADS1263_BusyDelayMs( UInt32 delayMs )
{
    SYSTICK_DelayMs( delayMs );
}

static void ADS1263_TaskDelayMs( UInt32 delayMs )
{
    TickType_t xDelayTicks;

    if( delayMs == 0U )
    {
        return;
    }

    xDelayTicks = pdMS_TO_TICKS( delayMs );
    if( xDelayTicks == 0U )
    {
        xDelayTicks = 1U;
    }

    vTaskDelay( xDelayTicks );
}

float ADS1263_GetTemperature( UInt8 ucCh )
{
    return ADS1263_GetTemperatureWithDelay( ucCh, ADS1263_BusyDelayMs );
}

float ADS1263_GetTemperatureTask( UInt8 ucCh )
{
    return ADS1263_GetTemperatureWithDelay( ucCh, ADS1263_TaskDelayMs );
}

static float ADS1263_GetTemperatureWithDelay( UInt8 ucCh, ADS1263_DelayFunc delayFunc )
{
    int32_t adcCode;
    float cjTemp;
    float tcTemp;
    float retTemp = 0.0;

    if( delayFunc == (ADS1263_DelayFunc)0 )
    {
        delayFunc = ADS1263_BusyDelayMs;
    }

    /* VBIAS(floating TC 공통모드 레벨시프트) 반영 */
    tc_apply_power();

    /* CJ(냉접점) 계측: ADS1263 내부 온도센서(다이온도) 사용
     * (외부 NTC 변환/핀페어 모호성 회피, 122.4mV@25°C·420µV/°C) */
    ADS1263_SetChannel(0x0B, 0x0B);             // INPMUX=0xBB 내부 온도센서
    ADS1263_WriteReg( ADS1263_MODE2, 0x0A );    // gain1
    ADS1263_StartAdc();                         // 계측 시작
    delayFunc( 50U );
    adcCode = ADS1263_ReadAdc();                // 계측 데이터 획득
    cjTemp = 25.0f + ( ADC_CodeToVoltage(adcCode)*1.0e6f - 122400.0f ) / 420.0f;

    if( ucCh == 0 )
    {
        /* TC 계측 */
        ADS1263_SetChannel(0, 1);
        s_curTcOffmV = ADS1263_GetTcOffsetCh( s_tcDev, 0 );
        ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );
        ADS1263_StartAdc();
        delayFunc( 50U );
        adcCode = ADS1263_ReadAdc();                // 계측 데이터 획득
        tcTemp = adc_to_tc_temp2( adcCode, cjTemp );
        retTemp = tcTemp;
    }
    else if( ucCh == 1 )
    {
        /* TC 계측 */
        ADS1263_SetChannel(2, 3);
        s_curTcOffmV = ADS1263_GetTcOffsetCh( s_tcDev, 2 );
        ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );
        ADS1263_StartAdc();
        delayFunc( 50U );
        adcCode = ADS1263_ReadAdc();                // 계측 데이터 획득
        tcTemp = adc_to_tc_temp2( adcCode, cjTemp );
        retTemp = tcTemp;
    }
    else if( ucCh == 2 )
    {
        /* TC 계측 */
        ADS1263_SetChannel(4, 5);
        s_curTcOffmV = ADS1263_GetTcOffsetCh( s_tcDev, 4 );
        ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );
        ADS1263_StartAdc();
        delayFunc( 50U );
        adcCode = ADS1263_ReadAdc();                // 계측 데이터 획득
        tcTemp = adc_to_tc_temp2( adcCode, cjTemp );
        retTemp = tcTemp;
    }
    else if( ucCh == 3 )
    {
        /* TC 계측 */
        ADS1263_SetChannel(6, 7);
        s_curTcOffmV = ADS1263_GetTcOffsetCh( s_tcDev, 6 );
        ADS1263_WriteReg( ADS1263_MODE2, tc_mode2_val() );
        ADS1263_StartAdc();
        delayFunc( 50U );
        adcCode = ADS1263_ReadAdc();                // 계측 데이터 획득
        tcTemp = adc_to_tc_temp2( adcCode, cjTemp );
        retTemp = tcTemp;
    }
    else if( ucCh == 4 )
    {
        /* CJ 계측 */
        retTemp = cjTemp;
    }
    else
    {
        /* code */
    }

    /* [진단] 이 채널의 마지막 raw 저장 (adcCode = CH0~3이면 TC코드, CH4면 CJ코드).
     * 0xFFFFFFFF면 SPI 무응답 = 칩이 DOUT을 안 잡음(하드웨어). 코드/채널매핑 무관. */
    if( ucCh <= 4U ) { s_tcRawCode[(s_tcDev==2)?1:0][ucCh] = adcCode; }

    return retTemp;
}
