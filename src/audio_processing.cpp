#include <math.h>
#include <stdio.h>
#include <new>

#include "audio_processing.h"
#include "fixed_point_functions.h"
#include "limiter.h"
#include "gain_struct.h"

static Limiter *lim;

/******************************************************************************/

void new_handler(void)
{
    printf("new: not enough memory!\n");
    exit(1);
}

/******************************************************************************/

void init_audio_processing(void)
{
    // char *x = new char[100000];
    // printf("x = %x\n", (int) x);

    lim = new Limiter(48000, 0.002f, 0.000f, 0.5f);
}
//------------------------------------------------------------------------------

gains audio_processing(int32_t input_samples[], int num_in_chans,
                      int32_t output_samples[], int num_out_chans)
{
    gains gain;
    gain = lim->process_sample(input_samples, output_samples);
    return gain;
}

//------------------------------------------------------------------------------

