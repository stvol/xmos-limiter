/******************************************************************************\
 * Header:  audio_stream
 * File:    audio_stream.h
 *
 * Description: Definitions, types, and prototypes for audio_stream.xc
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

#ifndef _AUDIO_STREAM_H_
#define _AUDIO_STREAM_H_

#include <xs1.h>
#include <print.h>
#include "app_global.h"
#include "app_conf.h"
#include "gain_struct.h"

#ifdef USE_XSCOPE
#include <xscope.h>
#endif // ifdef USE_XSCOPE

void audio_stream( // Coar that applies BiQuad filter to stream of audio samples
    streaming chanend c_dsp_gain // Channel connecting to DSP-control coar (bi-directional)
);

/******************************************************************************/

#endif // _AUDIO_STREAM_H_
/******************************************************************************/
// audio_stream.h
