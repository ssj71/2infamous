//spencer jackson
//rms_calc.h - an rms calculation library thing.
#include<stdint.h>

typedef struct _RMS_CALC
{
    uint16_t size;
    uint16_t indx;
    float* buf;
    float sum;
    float rms;
} rms_calc_t;

void rms_init(rms_calc_t* calc, uint16_t size);
void rms_cleanup(rms_calc_t* calc);
float rms_shift(rms_calc_t* calc, float x);
void rms_shift_no_out(rms_calc_t* calc, float x);
float rms_calculate(rms_calc_t* calc);
float rms_block_fill(rms_calc_t* calc, float x[], uint32_t nframes);
