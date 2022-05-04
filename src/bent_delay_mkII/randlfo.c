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
    
    //const vars
    lfo->ro = 2*sample_rate/fragsize;
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
    //this is a LPF with cutoff at freq
    float r = 3*2.0*rand() / (float)RAND_MAX -1.0;//extra 3 factor because lpf seems to need a bit more gain
    float a = 2.0*PI*freq;
    float b = 1.0/(lfo->ro+a);
    float y0 = a*b*(lfo->x1 + r - lfo->y1) + lfo->ro*b*lfo->y1;
    //TODO: clamp to [-1,1]?
    //store memory
    lfo->y1 = y0;
    lfo->x1 = r;
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
