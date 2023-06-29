    //Spencer Jackson
//apc.c
#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
#define CLAMP(X,MIN,MAX) X = X<MIN?MIN:X>MAX?MAX:X

#define BLOCKMASK 0xff

typedef struct _APC
{
    float phase;
    float lastout;
    float nextrig;
    float sample_rate;

    float *out_p;
    float *on_p;
    float *f_p;
    float *pw_p;
    float *vol_p;
} plug_t;

void run_apc(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;
    const float slope = 1.0; //number of samples to ramp between high and low

    float trigperiod = plug->sample_rate/(20*powf(2.0, 0.099**plug->f_p));
    float pulsexpire = plug->sample_rate/(20*powf(2.0, 1.0 + 0.099**plug->pw_p));

    float out = plug->lastout;
    float phase = plug->phase;
    float nextrig = plug->nextrig;
    float vol = *plug->vol_p/100.0;

    //phase is always measured relative to the beginning of wave cycle
    // the waveform is /‾\__
    //states: high, low, transitioning up, transitioning down
    if(*plug->on_p)
    {
        for (uint16_t i=0;i<nframes;)
        {
            if(phase >= nextrig)
            {
                //rise
                out = (phase - nextrig)*2.0/slope - 1.0;
                phase += 1.0;
                if(out >= 1.0)
                {
                    out = 1.0;
                    //reset phase
                    phase -= nextrig;
                    nextrig = trigperiod;
                }
                plug->out_p[i++] = vol*out;
            }
            else if(out > -1.0 && phase >= pulsexpire)
            {
                //fall
                out = (pulsexpire - phase)*2.0/slope - 1.0;
                phase += 1.0;
                if(out <= -1.0)
                    out = -1.0;
                plug->out_p[i++] = vol*out;
            }
            else if(out == 1.0)
            {
                //we are waiting for the pulse to expire
                uint16_t n = ceil(pulsexpire - phase + i);
                if(n > nframes)
                    n = nframes;
                for(;i<n;i++)
                    plug->out_p[i] = vol*out;
                phase += n;
                //propogate nextrig in case pulse covers multiple
                while(phase >= nextrig)
                {
                    nextrig += trigperiod;
                }
                //if not at end of block, next sample should be the fall
            }
            else if(out == -1.0)
            {
                //we are waiting for the next trigger
                uint16_t n = ceil(nextrig - phase + i);
                if(n > nframes)
                    n = nframes;
                for(;i<n;i++)
                    plug->out_p[i] = vol*out;
                phase += n;
                //if not at end of block, next sample should rise
            }
            else
            {
                fprintf(stderr,"PANIC! ");
            }
        }
    }
    else
    {
        for (uint16_t i=0;i<nframes;i++)
            plug->out_p[i] = 0.0;
    }
        


    plug->lastout = out;
    plug->phase = phase;
    plug->nextrig = nextrig;

    return;
}

LV2_Handle init_apc(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    plug_t* plug = malloc(sizeof(plug_t));

    plug->sample_rate = sample_rate;
    plug->phase = 0.0;
    plug->lastout = -1.0; //start in low state
    plug->nextrig = plug->sample_rate/20.0;

    return plug;
}

void connect_apc_ports(LV2_Handle handle, uint32_t port, void *data)
{
    plug_t* plug = (plug_t*)handle;
    switch(port)
    {
        PORT_CONNECT(0,out_p);
        PORT_CONNECT(1,on_p);
        PORT_CONNECT(2,f_p);
        PORT_CONNECT(3,pw_p);
        PORT_CONNECT(4,vol_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

void cleanup_apc(LV2_Handle handle)
{
    plug_t* plug = (plug_t*)handle;
    free(plug);
}

static const LV2_Descriptor apc_descriptor=
{
    "http://ssj71.github.io/infamousPlugins/plugs.html#ataripunkconsole",
    init_apc,
    connect_apc_ports,
    0,//activate
    run_apc,
    0,//deactivate
    cleanup_apc,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &apc_descriptor;
    default:
        return 0;
    }
}
