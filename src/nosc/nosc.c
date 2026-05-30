//spencer
//noscillator

#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
//TODO: def PI

typedef struct _noscillator
{
    float phase;
    float prev;
    float samplerate
}

typedef struct _NOSC
{
    float outgn;
    float dgn;

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
    x2 = modf(x+PI,2*PI) - PI;
    float y = 1.27323954474*x - 0.40528473456*x*(x>0?x:-x); //these are 4/pi and 4/pi/pi
    return 0.225*(y*(y>0?y:-y) - y) + y; //this is 9/40
}

float nosc_out(float phase, float prev, float rich, float hollow, float smooth)
{
    return nsin(phase + smooth*nsin(hollow*phase) + rich*prev);
}

float nosc_next(nosc* self, float freq, float rich, float hollow, float smooth)
{
    const float phase_step = 2*PI*freq/self->samplerate;
    self->prev = nosc_out(self->phase, self->prev, rich, hollow, smooth);
    self->phase += phase_step;
    return self->prev;
}

void nosc_phase_wrap(nosc* self)
{
}

void nosc_run(nosc_t *nosc, float* in, float* out, float* vfreq, uint32_t nframes, float oct, float cent, float warm, float rich, float hollow, float smooth)
{
    //0v is middle C midi 60 261.63hz
    const float middlec = 261.6255653005986;
    float prevfreq = vfreq[0] + 1.0; //force first calculation
    float prevfhz = 0;
    
    for(i = 0; i<nframes, i++)
    {
        if(vfreq[i] == prevfreq)
        {
            fhz = prevfhz;
        }
        else
        {
            fhz = middlec*powf(2,vfreq[i] + oct + cent/100);
        }
        prevfhz = fhz;
        prevfreq = vfreq[i]

        fhz *=.05*warm*randlfo_out();

        out[i] = nosc_next(nosc, fhz, rich, hollow, smooth);
    }
    
    nosc->phase = modf(nosc->phase,2*PI);
}

void nosc_plugin_run(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;
    nosc_run(&plug->nosc, plug->in_p, plug->out_p, plug->freq_p, nframes, *plug->oct_p, *plug->cent_p, *plug->warm_p, *plug->rich_p, *plug->hollow_p, *plug->smooth_p);
}

void nosc_connect_ports(LV2_Handle handle, uint32_t port, void *data)
{
    plug_t* plug = (plug_t*)handle;
    switch(port)
    {
        PORT_CONNECT(0,in_p);
        PORT_CONNECT(1,out_p);
        PORT_CONNECT(2,freq_p);
        PORT_CONNECT(3,on_p);
        PORT_CONNECT(4,_p);
        PORT_CONNECT(5,_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

LV2_Handle init_nosc(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    //TODO:
    plug_t* plug = malloc(sizeof(plug_t));
}
