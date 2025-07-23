//Spencer Jackson
//bent_delay.c
#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include"randlfo.h"

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
#define CLAMP(X,MIN,MAX) X = X<MIN?MIN:(X>MAX?MAX:X)

#define BLOCKMASK 0xff

typedef struct _delayer
{
    uint16_t w; //write index
    uint16_t dly;
    float dstep;
    float fb;
    float fbstep;
    float *buf;
    float sample_rate;
    float wet;
    float dry;
    float wstep;
    float drystep;
    float fb2;
    float fb2step;
    float lvl;

    randlfo_t dlfo;
    randlfo_t fblfo;
} delayer_t;

typedef struct _BENTDELAY_MK_II
{
    delayer_t *delayer;

    float *in_p;
    float *out_p;
    float *outr_p;
    float *on_p;
    float *dly_p;
    float *drange_p;
    float *dfreq_p;
    float *fb_p;
    float *fbrange_p;
    float *fbfreq_p;
    float *mix_p;
    float *sense_p;
    float *stereo_p;
} plug_t;

//cubic interp, using the fact that the index will wrap around perfectly for a uint16_t
float cubic(float* buf, float i)
{
    uint16_t ii = (uint16_t)i-1;
    float a = buf[ii++];
    float x = i - ii;
    float b = buf[ii++];
    float c = buf[ii++];
    float d = buf[ii];
    float out =  b + 0.5 * x*(c - a + x*(2.0*a - 5.0*b + 4.0*c - d + x*(3.0*(b - c) + d - a)));
    return out;

}

void run_delayer(delayer_t* delayer, float* in, float* out, uint16_t nframes, float on, float mix, float fbl, float fbh, float fbf, float delay, float drange, float df, float thresh)
{
    float j,m,m2;
    double tmp;
    float* buf = delayer->buf;
    uint16_t w = delayer->w;
    float fb = delayer->fb;
    float fbstep = delayer->fbstep;
    float fb2 = delayer->fb2;
    float fb2step = delayer->fb2step;
    float dly = delayer->dly;
    float dstep = delayer->dstep;
    float wet = delayer->wet;
    float dry = delayer->dry;
    float wstep = delayer->wstep;
    float drystep = delayer->drystep;
    float lvl = delayer->lvl;

    CLAMP(fbl, -1.0, 1.0);
    CLAMP(fbh, -1.0, 1.0);

    for (uint16_t i=0;i<nframes;i++)
    {
        //fractional delay
        j = (uint16_t)(w-dly) + modf(w-dly,&tmp);
        m = cubic(buf,j);
        //half time delay
        j = (uint16_t)(w-dly/2.0) + modf(w-dly/2.0,&tmp);
        m2 = cubic(buf,j);
        buf[w] = on*in[i] + fb*m;
        out[i] = dry*in[i] + wet*(m + fb2*m2);
        dly += dstep;
        fb += fbstep;
        fb2 += fb2step;
        wet += wstep;
        dry += drystep;
        w++;
        //lvl = 0.00007*lvl + 0.99993*fabs(in[i]); //approx. VU meter
        j = fabs(in[i]);
        if(j > lvl)
            lvl = j;
        else
            lvl *= 0.99999;
        if(!(w&BLOCKMASK))
        {
            //new block, new lfo out
            delayer->fb = ((fbh - fbl)*randlfo_out(&delayer->fblfo,fbf) + (fbh+fbl))/2.0;
            CLAMP(delayer->fb, -1.0, 1.0);
            fbstep = (delayer->fb - fb)/(float)(BLOCKMASK+1.0);
            delayer->dly = (drange*randlfo_out(&delayer->dlfo,df) + delay)*delayer->sample_rate/1000.0;
            CLAMP(delayer->dly, 0, 0xffff);
            dstep = (delayer->dly - dly)/(float)(BLOCKMASK+1.0);
            if(mix <= .5)
            {
                delayer->wet = 2.0*mix;
                delayer->dry = 1.0;
            }
            else
            {
                delayer->wet = 1.0;
                delayer->dry = 2.0*(1.0-mix);
            }
            if(lvl > thresh)
            {
                delayer->fb2 = delayer->fb;
            }
            else
            {
                delayer->fb2 = 0.0;
            }
            fb2step = (delayer->fb2 - fb2)/(float)(BLOCKMASK+1.0);
            wstep = (delayer->wet - wet)/(float)(BLOCKMASK+1.0);
            drystep = (delayer->dry - dry)/(float)(BLOCKMASK+1.0);
        }
    } 

    delayer->w = w;
    delayer->fb = fb;
    delayer->fbstep = fbstep;
    delayer->fb2 = fb2;
    delayer->fb2step = fb2step;
    delayer->dly = dly;
    delayer->dstep = dstep;
    delayer->wet = wet;
    delayer->dry = dry;
    delayer->wstep = wstep;
    delayer->drystep = drystep;
    delayer->lvl = lvl;

    return;
}


void run_bent_delay(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;

    run_delayer(
        plug->delayer,
        plug->in_p,
        plug->out_p,
        nframes,
        *plug->on_p>0.0?1.0:0.0,
        *plug->mix_p,
        *plug->fb_p - *plug->fbrange_p,
        *plug->fb_p + *plug->fbrange_p,
        *plug->fbfreq_p,
        *plug->dly_p,
        *plug->drange_p,
        *plug->dfreq_p,
	    1.0-*plug->sense_p);
    if(plug->stereo_p)
        run_delayer(
            &plug->delayer[1],
            plug->in_p,
            plug->outr_p,
            nframes,
            *plug->on_p>0.0?1.0:0.0,
            *plug->mix_p,
            *plug->fb_p - *plug->fbrange_p,
            *plug->fb_p + *plug->fbrange_p,
            *plug->fbfreq_p,
            *plug->dly_p*3.0/(3.0-*plug->stereo_p),
            *plug->drange_p,
            *plug->dfreq_p,
            1.0-*plug->sense_p);
    return;
}

LV2_Handle init_bent_delay(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    uint32_t tmp;

    plug_t* plug = malloc(sizeof(plug_t));
    uint8_t l = strlen(descriptor->URI)>65?1:2;
    plug->delayer = malloc(sizeof(delayer_t)*l);

    tmp = 0x10000;
    for(l--;l<255;l--)
    {
        plug->delayer[l].buf = (float*)malloc(tmp*sizeof(float));
        for(tmp=0;tmp<0x10000;tmp++)
            plug->delayer[l].buf[tmp] = 0.0;
        plug->delayer[l].w = 0;
        plug->delayer[l].fb = 0;
        plug->delayer[l].fbstep = 0;
        plug->delayer[l].dly = 0;
        plug->delayer[l].dstep = 0;
        plug->delayer[l].wet = 0;
        plug->delayer[l].wstep = 0;
        plug->delayer[l].dry = 0;
        plug->delayer[l].drystep = 0;
        plug->delayer[l].lvl = 0;
        plug->delayer[l].sample_rate = sample_rate;

        randlfo_init(&plug->delayer[l].dlfo, sample_rate, BLOCKMASK+1);
        randlfo_init(&plug->delayer[l].fblfo, sample_rate, BLOCKMASK+1);
    }

    plug->stereo_p = NULL;

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
        PORT_CONNECT(9,mix_p);
        PORT_CONNECT(10,sense_p);
        PORT_CONNECT(11,stereo_p);
        PORT_CONNECT(12,outr_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

void cleanup_bent_delay(LV2_Handle handle)
{
    plug_t* plug = (plug_t*)handle;
    free(plug->delayer[0].buf);
    if(plug->stereo_p)
    {
        free(plug->delayer[1].buf);
    }
    free(plug->delayer);
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
static const LV2_Descriptor bent_delay_mono_descriptor=
{
    "http://ssj71.github.io/infamousPlugins/plugs.html#bent_delay_mkII_mono",
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
    case 1:
        return &bent_delay_mono_descriptor;
    default:
        return 0;
    }
}
