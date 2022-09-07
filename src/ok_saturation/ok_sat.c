    //Spencer Jackson
//ok_sat.c
#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
#define CLAMP(X,MIN,MAX) X = X<MIN?MIN:X>MAX?MAX:X

typedef struct _OK_SAT
{
    float gn; //gain, which approximately models temperature, sitting at 1.0 when below the rails, but heating up when the signal goes above the threshold, reducing gain

    float *in_p;
    float *out_p;
    float *sat_p;
    float *test_p;
    float *test2_p;
} plug_t;


void run_ok_sat(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;
    float gn = plug->gn;
    const float thresh = .61-.6**plug->sat_p;
    const float filt = .8; //speed of the "heating"
    const float filtdn = .9; //speed of the "heating" on the lower half of the wave
    const float smash = 1.0+10.0**plug->sat_p; //gain reducton due to heat
    const float smashdn = smash/2.0; //gain reducton due to heat on the lower half of the wave

    const float gainout = *plug->test2_p;

    float *in = plug->in_p;
    float * out = plug->out_p;
    
    for (uint32_t i=0;i<nframes;i++)
    {
        out[i] = gn*in[i];
        if(in[i] > thresh)
        {
            // heat up, lowering gain
            gn = filt*gn + (1.0-filt)*(((smash-1.0)*thresh+in[i])/(in[i]*smash));
        }
        else if(in[i] < -thresh)
        {
            // heat up, lowering gain (though not as dramatically on this side of the signal)
            gn = filtdn*gn + (1.0-filtdn)*(((1.0-smashdn)*thresh+in[i])/(in[i]*smashdn));
        }
        else
        {
            //cool down (head back to unity)
            gn = filt*gn + (1.0-filt)*1.0;
        }
        out[i] *= gainout;
    }
    plug->gn = gn;

    return;
}

LV2_Handle init_ok_sat(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    plug_t* plug = malloc(sizeof(plug_t));
    plug->gn = 1.0;
    return plug;
}

void connect_ok_sat_ports(LV2_Handle handle, uint32_t port, void *data)
{
    plug_t* plug = (plug_t*)handle;
    switch(port)
    {
        PORT_CONNECT(0,in_p);
        PORT_CONNECT(1,out_p);
        PORT_CONNECT(2,sat_p);
        PORT_CONNECT(3,test_p);
        PORT_CONNECT(4,test2_p);
        PORT_CONNECT(5,outctl_p);
    default:
        puts("UNKNOWN PORT YO!!");
    }
}

void cleanup_ok_sat(LV2_Handle handle)
{
    plug_t* plug = (plug_t*)handle;
    free(plug);
}

static const LV2_Descriptor ok_sat_descriptor=
{
    "http://ssj71.github.io/infamousPlugins/plugs.html#ok_sat",
    init_ok_sat,
    connect_ok_sat_ports,
    0,//activate
    run_ok_sat,
    0,//deactivate
    cleanup_ok_sat,
    0//extension
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    switch (index)
    {
    case 0:
        return &ok_sat_descriptor;
    default:
        return 0;
    }
}
