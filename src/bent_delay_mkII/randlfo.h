//spencer jackson

//gpl v3 and all that

//a brownish noise lfo implementation

#include<stdint.h>

typedef struct randlfo_t_
{
    float y1,x1,ro; //memory for filters
}randlfo_t;

void randlfo_init(randlfo_t *lfo, double sample_rate, uint32_t fragsize);
float randlfo_out(randlfo_t *lfo, float freq);
void randlfo_reset(randlfo_t *lfo);
