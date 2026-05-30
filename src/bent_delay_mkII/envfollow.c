// spencer jackson
// gpl v3 and all that

// a little envelope follower

#include<math.h>
#include "envfollow.h"

#define PI 3.1415926535897932384626433832795

// aiming for fundamentals (or harmonics for bass)
#define HPF_CUTOFF_HZ 100.0
#define LPF_CUTOFF_HZ 2000.0

//single zero/pole high pass
float hpf_coeff(double sample_rate, float cutoff_hz)
{
    return 1.0 / (1.0 + (2 * PI * cutoff_hz / sample_rate));
}

void hpf_init(hpf_t *hpf, double sample_rate, float cutoff_hz)
{
    hpf->r = hpf_coeff(sample_rate, cutoff_hz);
    hpf->x1 = 0.0;
    hpf->y1 = 0.0;
}

float hpf_out(hpf_t *hpf, float x)
{
    float y = hpf->r * ( hpf->y1 + x - hpf->x1);
    hpf->x1 = x;
    hpf->y1 = y;
    return y;
}

//for single pole IIR
float lpf_coeff(double sample_rate, float cutoff_hz)
{
    return exp(-2 * PI * cutoff_hz / sample_rate);
}

void lpf_init(lpf_t *lpf, double sample_rate, float cutoff_hz)
{
    lpf->a = lpf_coeff(sample_rate, cutoff_hz);
    lpf->y1 = 0.0;
}

float lpf_out(lpf_t *lpf, float x)
{
    float y = (1.0 - lpf->a) * x + lpf->a * lpf->y1;
    lpf->y1 = y;
    return y;
}

void  envfollow_init(envfollow_t *follower, double sample_rate)
{
    hpf_init(&follower->hpf, sample_rate, HPF_CUTOFF_HZ);
    lpf_init(&follower->lpf, sample_rate, LPF_CUTOFF_HZ);
    follower->o = 0.0;
}

float envfollow_out(envfollow_t *follower, float x)
{
    float xh = hpf_out(&follower->hpf, x);
    float xl = lpf_out(&follower->lpf, xh);
    if(xl > follower->o)
        follower->o = xl;
    else
        follower->o *= 0.99999;
    return follower->o;
}
