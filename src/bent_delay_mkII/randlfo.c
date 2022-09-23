//spencer jackson

//gpl v3 and all that

//a brownish noise lfo implementation
#include<stdlib.h>
#include<time.h>
#include"randlfo.h"

#define PI 3.1415926535897932384626433832795

/**
* @brief init rand lfo
*
* @param lfo handle
* @param sample_rate hz of audio data
* @param fragsize block size in samples that LFO will be recalculated, typically around 6ms (256)
*/
void randlfo_init(randlfo_t *lfo, double sample_rate, uint32_t fragsize)
{
    //init states
    srand ((unsigned int) time (NULL));
    lfo->y1 = lfo->x1 = 0;
    lfo->y2 = lfo->x2 = 0;
    
    //const vars
    lfo->ro = sample_rate/fragsize;
}


//frg 256 smp ~= 6ms


/**
* @brief calculate another sample of the waveform, this is intended to be run every few ms
*
* @param lfo handle
* @param freq filter cutoff frequency (hz), should approximately be what freq you'd want a sine lfo
*
* @return sample from lfo
*/
float randlfo_out(randlfo_t *lfo, float freq)
{
    //this is a resonant LPF with cutoff at freq
    const float x0 = 2.0*(rand() / (float)RAND_MAX) -1.0;
    const float z = .0000001; //damping factor E(0-1]
    const float w = 2.0*PI*freq;
    const float T = 1.0/lfo->ro;
    float g = 60.0/freq; //this is a makeup gain because as the filter cuts off more and more energy the amplitude drops.
    //if using this for frequencies higher than 10hz you may need to tweak the numerator
    if(g > 100000)g=100000;

    const float a = w*T;
    const float b = a*a;
    const float c = 4.0*z*a;
    const float d = 1.0/(b + c + 4.0);
    const float y0 = d*( b*(g*x0 + 2.0*lfo->x1 + lfo->x2) - (b+b-8.0)*lfo->y1 - (b-c+4.0)*lfo->y2);
    //TODO: clamp to [-1,1]?
    //store memory
    lfo->y1 = y0;
    lfo->x1 = x0;
    lfo->y2 = lfo->y1;
    lfo->x2 = lfo->x1;
    if(y0> 1.0 || y0 < -1.0)
    fprintf(stderr, "%f, ", y0);
    return y0;
}

/**
* @brief resets internal parameters of LFO
*
* @param lfo handle
*/
void randlfo_reset(randlfo_t *lfo)
{
    lfo->y1 = lfo->x1 = 0;
}
