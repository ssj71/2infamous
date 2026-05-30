// spencer jackson
// gpl v3 and all that

// a little envelope follower

#include<stdlib.h>

typedef struct hpf_t_
{
    float r,x1,y1;
}hpf_t;

typedef struct lpf_t_
{
    float a,y1;
}lpf_t;

typedef struct envfollow_t_
{
    hpf_t hpf;
    lpf_t lpf;
    float o;
}envfollow_t;

void envfollow_init(envfollow_t *follower, double sample_rate);
