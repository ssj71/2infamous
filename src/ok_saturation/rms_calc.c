//spencer jackson
//rms_calc.c - an rms calculation library thing.
#include<math.h>
#include<stdlib.h>
#include<stdint.h>
#include"rms_calc.h"

void rms_init(rms_calc_t* calc, uint16_t size)
{
    uint16_t i;
    calc->buf = (float*)malloc(sizeof(float)*size);
    calc->size = size;
    calc->indx = 0;
    calc->sum = 0;
    calc->rms = 0;
    for(i=0; i<size; i++)
        calc->buf[i] = 0;
}

void rms_cleanup(rms_calc_t* calc)
{
    free(calc->buf);
}

void rms_shift_no_out(rms_calc_t* calc, float x)
{
    calc->sum -= calc->buf[calc->indx];
    calc->buf[calc->indx] = x*x;
    calc->sum += calc->buf[calc->indx++];
    calc->indx = calc->indx<calc->size?calc->indx:0;
    calc->sum = calc->sum>=0?calc->sum:0;
}

float rms_calculate(rms_calc_t* calc)
{
    return calc->rms = sqrt(calc->sum/(float)calc->size);
}

float rms_shift(rms_calc_t* calc, float x)
{
    rms_shift_no_out(calc,x);
    return rms_calculate(calc);
}

float rms_block_fill(rms_calc_t* calc, float x[], uint32_t nframes)
{
    uint32_t i;
    for(i=0; i<nframes; i++)
    {
        rms_shift_no_out(calc,x[i]);
    }
    return rms_calculate(calc);
}
