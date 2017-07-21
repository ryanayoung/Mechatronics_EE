#include "direction.h"

const int thresh = 5;   // Threshold amplitude for detecting a ping.
const int hDist = 10;   // Pitch between hydrophones.
const int sRate = 5000; // ADC sampling rate.
const int speedOfSound = 1482;

const double angleLUT[9] =
{
    0,
    6.427531,
    12.93778,
    19.62358,
    26.6017,
    34.03727,
    42.19674,
    51.59367,
    63.58188
};
