//spencer jackson

//gpl v3 and all that

//a brownish noise lfo implementation
//this provides smooth random values that approximate modulating a sine wave with random
//amplitude every half cycle. Values returned in range [-1,1]. The random values can be
//a uniform distribution or a normal distribution (more of a pyramid than bell curve)
//It also provides API for calling every sample or once per block (256 samples)

#include<stdint.h>
#include<stdbool.h>

typedef struct randlfo_t_
{
    float c,y1,x1,ro; //memory for filters
    uint8_t blk;
    bool normal;
}randlfo_t;

void randlfo_init(randlfo_t *lfo, double sample_rate, bool normal);
float randlfo_out(randlfo_t *lfo, float freq);
void randlfo_reset(randlfo_t *lfo);

#ifdef RANDLFO_IMPLEMENTATION

#include<stdlib.h>
#include<time.h>

#define PI 3.1415926535897932384626433832795

/**
* @brief init rand lfo
*
* @param lfo handle
* @param sample_rate hz of audio data
* @param normal if true output is weighted to be more like normal distribution, if false output is uniform distribution
*/
void randlfo_init(randlfo_t *lfo, double sample_rate, bool normal)
{
    const uint32_t blocksize = 256; //block size in samples that LFO will be recalculated, typically around 6ms (256)
    //init states
    srand ((unsigned int) time (NULL));
    lfo->y1 = lfo->x1 = 0.0;
    lfo->c = 0;
    lfo->blk = 0;
    
    //const vals
    lfo->ro = sample_rate/blocksize;
    lfo->normal = normal;
}


//frg 256 smp ~= 6ms


/**
* @brief calculate the next value of the waveform, skipping to the next block, this should only be called once per block (every 256 samples)
* @param lfo handle
* @param freq filter cutoff frequency (hz), should approximately be what freq you'd want a sine lfo
*
* @return sample from lfo
*/
float randlfo_out_blk(randlfo_t *lfo, float freq)
{
    float y0;
    const float end = lfo->ro/(2.0*freq);
    const float s = (lfo->x1 - lfo->y1)*16.0*freq*freq/lfo->ro/lfo->ro; //step
    //at audio rates I'd be worried about jitter but at LFO rates, we'll be fine
    if(++lfo->c >= end)
    {
        lfo->c = 0.0; //counter
        if(lfo->normal)
        {
            lfo->x1 = (rand() / (float)RAND_MAX) + (rand() / (float)RAND_MAX) -1.0; //input
        }
        else
        {
            lfo->x1 = 2.0*(rand() / (float)RAND_MAX) - 1.0; //input
        }
        y0 = lfo->y1;
    }
    else if(lfo->c > end/2.0)
    {
        y0 = lfo->y1 + (end-lfo->c)*s;
    }
    else
    {
        y0 = lfo->y1 + lfo->c*s;
    }

    //store memory
    lfo->y1 = y0;
    return y0;
}

/**
* @brief calculate another sample of the waveform, this is intended to be run every sample, but only changes value every block (256 samples)
*
* @param lfo handle
* @param freq filter cutoff frequency (hz), should approximately be what freq you'd want a sine lfo
*
* @return sample from lfo
*/
float randlfo_out(randlfo_t *lfo, float freq)
{
    if(lfo->blk++)
    {
        return lfo->y1;
    }
    return randlfo_out_blk(lfo, freq);
}

/**
* @brief resets internal parameters of LFO
*
* @param lfo handle
*/
void randlfo_reset(randlfo_t *lfo)
{
    lfo->y1 = lfo->x1 = 0;
    lfo->blk = 0;
}
#endif //RANDLFO_IMPLEMENTATION
