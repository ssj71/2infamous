//spencer
//noscillator

#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#define RANDLFO_IMPLEMENTATION
#include"randlfo.h"

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break

typedef struct _noscillator
{
    randlfo_t lfo;
    float phase; //E[-PI,PI]
    float prev;
    float samplerate;
}nosc_t;

typedef struct _NOSC
{
    nosc_t nosc;

    float *in_p;
    float *out_p;
    float *freq_p;
    float *on_p;
    float *rich_p;
    float *hollow_p;
    float *smooth_p;
    float *oct_p;
    float *cent_p;
    float *warm_p;
} plug_t;

float nsin(float x)
{
    while(x > PI)
    {
        x -= 2*PI;
    }
    while(x < -PI)
    {
        x += 2*PI;
    }
    float y = 1.27323954474*x - 0.40528473456*x*(x>0?x:-x); //these are 4/pi and 4/pi/pi
    return 0.225*(y*(y>0?y:-y) - y) + y; //this is 9/40
}

float nosc_out(float phase, float prev, float rich, float hollow, float smooth)
{
    return nsin(phase + smooth*nsin(hollow*phase) + rich*prev);
}

float nosc_next(nosc_t* self, float freq, float rich, float hollow, float smooth)
{
    const float phase_step = 2*PI*freq/self->samplerate;
    self->prev = nosc_out(self->phase, self->prev, rich, hollow, smooth);
    self->phase += phase_step;
    return self->prev;
}

void nosc_run(nosc_t *nosc, float* out, float* vfreq, uint32_t nframes, float oct, float cent, float warm, float rich, float hollow, float smooth)
{
    //0v is middle C, C3, midi 60, or 261.63hz
    const float middlec = 261.6255653005986;
    float prevfreq = vfreq[0] + 1.0; //force first calculation
    float prevfhz = 0;
    
    for(uint32_t i = 0; i<nframes; i++)
    {
        float fhz = 0;
        if(vfreq[i] == prevfreq)
        {
            fhz = prevfhz;
        }
        else
        {
            fhz = middlec*powf(2,vfreq[i] + oct + cent/1200.0);
        }
        prevfhz = fhz;
        prevfreq = vfreq[i];

        fhz *= 1.0 + .02*warm*randlfo_out(&nosc->lfo, .3);

        out[i] = nosc_next(nosc, fhz, rich, hollow, smooth);
    }
    while(nosc->phase > PI)
    {
        nosc->phase -= 2*PI;
    }
    while(nosc->phase < -PI)
    {
        nosc->phase += 2*PI;
    }
}

void nosc_plugin_run(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;
    if(*plug->on_p < 0.5)
    {
        memset(plug->out_p, 0, sizeof(float)*nframes);
        return;
    }
    nosc_run(&plug->nosc, plug->out_p, plug->freq_p, nframes, *plug->oct_p, *plug->cent_p, *plug->warm_p, *plug->rich_p, 2.0**plug->hollow_p, *plug->smooth_p);
}

void nosc_connect_ports(LV2_Handle handle, uint32_t port, void *data)
{
    plug_t* plug = (plug_t*)handle;
    switch(port)
    {
        PORT_CONNECT(0,out_p);
        PORT_CONNECT(1,freq_p);
        PORT_CONNECT(2,on_p);
        PORT_CONNECT(3,oct_p);
        PORT_CONNECT(4,cent_p);
        PORT_CONNECT(5,rich_p);
        PORT_CONNECT(6,hollow_p);
        PORT_CONNECT(7,smooth_p);
        PORT_CONNECT(8,warm_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

LV2_Handle init_nosc(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    plug_t* plug = malloc(sizeof(plug_t));

    plug->nosc.phase = 0.0;
    plug->nosc.prev = 0.0;
    plug->nosc.samplerate = (float)sample_rate;

    randlfo_init(&plug->nosc.lfo, sample_rate, true);

    return plug;
}

void nosc_cleanup(LV2_Handle handle)
{
    plug_t* plug = (plug_t*)handle;
    free(plug);
}

static const LV2_Descriptor nosc_descriptor=
{
    "http://ssj71.github.io/infamousPlugins/plugs.html#noscillator",
    init_nosc,
    nosc_connect_ports,
    0,//activate
    nosc_plugin_run,
    0,//deactivate
    nosc_cleanup,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &nosc_descriptor;
    default:
        return 0;
    }
}
