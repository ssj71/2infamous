    //Spencer Jackson
//bent_delay.c
#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include"randlfo.h"

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
#define CLAMP(X,MIN,MAX) X = X<MIN?MIN:X>MAX?MAX:X

#define BLOCKMASK 0xff

typedef struct _BENTDELAY_MK_II
{
    uint16_t w; //write index
    uint16_t dly;
    float dstep;
    float fb;
    float fbstep;
    float *buf;
    float sample_rate;

    randlfo_t dlfo;
    randlfo_t fblfo;

    float *in_p;
    float *out_p;
    float *on_p;
    float *dly_p;
    float *drange_p;
    float *dfreq_p;
    float *fbfreq_p;
    float *fb_p;
    float *fbrange_p;
} plug_t;


void run_bent_delay(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;

    float* in = plug->in_p;
    float* out = plug->out_p;
    const float byp = 1.0-*plug->on_p;
    float fbl = *plug->fb_p - *plug->fbrange_p;
    float fbh = *plug->fb_p + *plug->fbrange_p;

    float* buf = plug->buf;
    uint16_t w = plug->w;
    float fb = plug->fb;
    float fbstep = plug->fbstep;
    float dly = plug->dly;
    float dstep = plug->dstep;

    CLAMP(fbl, -1.0, 1.0);
    CLAMP(fbh, -1.0, 1.0);
    
    for (uint16_t i=0;i<nframes;i++)
    {
        out[i] = in[i] + buf[(uint16_t)(w-dly)];
        buf[w] = fb*(out[i] - byp*in[i]);
        dly += dstep;
        fb += fbstep;
        w++;
        if(!(w&BLOCKMASK))
        {
            //new block, new lfo out
            plug->fb = ((fbh - fbl)*randlfo_out(&plug->fblfo,*plug->fbfreq_p) + (fbh+fbl))/2.0;
            CLAMP(plug->fb, -1.0, 1.0);
            fbstep = (plug->fb - fb)/(float)(BLOCKMASK+1.0);
            plug->dly = (*plug->drange_p*randlfo_out(&plug->dlfo,*plug->dfreq_p) + *plug->dly_p)*plug->sample_rate/1000.0;
            CLAMP(plug->dly, 0, 0xffff);
            dstep = (plug->dly - dly)/(float)(BLOCKMASK+1.0);
        }
    } 

    plug->w = w;
    plug->fb = fb;
    plug->fbstep = fbstep;
    plug->dly = dly;
    plug->dstep = dstep;

    return;
}

LV2_Handle init_bent_delay(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    uint32_t tmp;

    plug_t* plug = malloc(sizeof(plug_t));

    tmp = 0x10000;
    plug->buf = (float*)malloc(tmp*sizeof(float));
    plug->w = 0;
    plug->fb = 0;
    plug->fbstep = 0;
    plug->dly = 0;
    plug->dstep = 0;

    randlfo_init(&plug->dlfo, sample_rate, BLOCKMASK+1);
    randlfo_init(&plug->fblfo, sample_rate, BLOCKMASK+1);

    plug->sample_rate = sample_rate;

    return plug;
}

void connect_bent_delay_ports(LV2_Handle handle, uint32_t port, void *data)
{
    plug_t* plug = (plug_t*)handle;
    switch(port)
    {
        PORT_CONNECT(0,in_p);
        PORT_CONNECT(1,out_p);
        PORT_CONNECT(2,on_p);
        PORT_CONNECT(3,dly_p);
        PORT_CONNECT(4,drange_p);
        PORT_CONNECT(5,dfreq_p);
        PORT_CONNECT(6,fb_p);
        PORT_CONNECT(7,fbrange_p);
        PORT_CONNECT(8,fbfreq_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

void cleanup_bent_delay(LV2_Handle handle)
{
    plug_t* plug = (plug_t*)handle;
    free(plug->buf);
    free(plug);
}

static const LV2_Descriptor bent_delay_descriptor=
{
    "http://ssj71.github.io/infamousPlugins/plugs.html#bent_delay_mkII",
    init_bent_delay,
    connect_bent_delay_ports,
    0,//activate
    run_bent_delay,
    0,//deactivate
    cleanup_bent_delay,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &bent_delay_descriptor;
    default:
        return 0;
    }
}
