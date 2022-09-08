    //Spencer Jackson
//ok_sat.c
#include<lv2.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include"rms_calc.h"

#define PORT_CONNECT(N,PORT) case N: plug->PORT = (float*)data; break
#define CLAMP(X,MIN,MAX) X = X<MIN?MIN:X>MAX?MAX:X
#define BLOCKMASK 0x01FF

typedef struct _OK_SAT
{
    float tgn; //"tube" gain, which approximately models temperature, sitting at 1.0 when below the rails, but heating up when the signal goes above the threshold, reducing gain
    uint16_t bc;
    float outgn;
    float dgn;
    float pin; //memory for dc rm filter for rms
    float pout; //memory for dc rm filter for rms

    float *in_p;
    float *out_p;
    float *sat_p;
    float *test_p;
    float *test2_p;
    float *outctl_p;

    rms_calc_t prerms;
    rms_calc_t postrms;
} plug_t;


void run_ok_sat(LV2_Handle handle, uint32_t nframes)
{
    plug_t* plug = (plug_t*)handle;
    float gn = plug->tgn;
    float gainout = plug->outgn;
    float dgain = plug->dgn;
    float pin = plug->pin;
    float pout = plug->pout;
    uint16_t blockcount = plug->bc;
    const float sat = -*plug->sat_p**plug->sat_p + 2**plug->sat_p; //parabolic mapping
    const float thresh = .401-.4*sat;
    const float filt = .8; //speed of the "heating"
    const float filtdn = .9; //speed of the "heating" on the lower half of the wave
    const float smash = 1.0+11.0*sat; //gain reducton due to heat
    const float smashdn = smash/2.0; //gain reducton due to heat on the lower half of the wave


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
        //now try to keep the level the same
        //dc rm filter on input rms
        pout = in[i] - pin + .99*pout;
        pin = in[i];
        if(!(++blockcount&BLOCKMASK))
        {
            //g = i/o
            //dc offset in input screws with this badly
            const float pre = rms_shift(&plug->prerms, pout);
            const float pst = rms_shift(&plug->postrms, out[i]);
            if(pst)
            {
                dgain = (pre/pst - gainout)/(float)(BLOCKMASK+1);
            }
            else
            {
                //don't div by 0
                dgain = 0.0;
            }
            CLAMP(dgain,-.001,.001);
            //fprintf(stderr,"in %f b %f a %f gn %f d %f\n",in[i], pre, pst,gainout,dgain);
        }
        else
        {
            rms_shift_no_out(&plug->prerms, pout);
            rms_shift_no_out(&plug->postrms, out[i]);
        }
        gainout += dgain;
        out[i] *= gainout;
    }
    plug->tgn = gn;
    plug->outgn = gainout;
    plug->dgn = dgain;
    plug->bc = blockcount;
    plug->pout = pout;
    plug->pin = pin;
    *plug->outctl_p = gainout; //display for test

    return;
}

LV2_Handle init_ok_sat(const LV2_Descriptor *descriptor,double sample_rate, const char *bundle_path,const LV2_Feature * const* host_features)
{
    plug_t* plug = malloc(sizeof(plug_t));
    plug->tgn = 1.0;
    plug->bc = 0.0;
    plug->outgn = 1.0;
    plug->dgn = 0.0;
    plug->pin = 0.0;
    plug->pout = 0.0;
    rms_init(&plug->prerms, (BLOCKMASK+1)<<2);
    rms_init(&plug->postrms, (BLOCKMASK+1)<<2);
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
    rms_cleanup(&plug->prerms);
    rms_cleanup(&plug->postrms);
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
