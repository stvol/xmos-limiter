/******************************************************************************\
 * File:	audio_stream.xc
 *  
 * Description: Coar that applies BiQuad filter to stream of audio samples
 *
 * Version: 0v1
 * Build:
 *
 * The copyrights, all other intellectual and industrial
 * property rights are retained by XMOS and/or its licensors.
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2012
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the
 * copyright notice above.
 *
\******************************************************************************/

#include "audio_stream.h"
#include "audio_processing.h"

static int32_t inp_samps[NUM_APP_CHANS];   // Input audio sample buffer
static int32_t out_samps[NUM_APP_CHANS];   // Output audio sample buffer

// DSP-control coar.

/******************************************************************************/
void audio_stream( // Coar that applies a BiQuad filter to a set of of audio sample streams
    streaming chanend c_dsp // DSP end of channel connecting to Audio_IO and DSP coars (bi-directional)
)
{
    // NB Setup correct number of channels in Makefile

    int chan_cnt; // Channel counter

    // Loop forever
    while(1)
    {
        // Send/Receive samples over Audio coar channel
#pragma loop unroll
        for (chan_cnt = 0; chan_cnt < NUM_APP_CHANS; chan_cnt++)
        {
            c_dsp :> inp_samps[chan_cnt]; 
            c_dsp <: out_samps[chan_cnt];
        }
        gains gains;
        gains = audio_processing(inp_samps, NUM_APP_CHANS, out_samps, NUM_APP_CHANS);
        //xscope_int(0,gains.normal_gain);
        //xscope_int(1,gains.crest_gain);
        //xscope_int(2,gains.crest_fac);
        xscope_int(0,gains.tau);

    } // while(1)

} // audio_stream
/*****************************************************************************/
// audio_stream.xc
