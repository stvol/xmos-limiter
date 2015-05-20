#ifndef _AUDIO_PROCESSING
#define _AUDIO_PROCESSING

#include <stdint.h>
#include "app_conf.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC
void init_audio_processing(void);

EXTERNC
int32_t audio_processing(int32_t input_samples[NUM_APP_CHANS], int num_in_chans,
                      int32_t output_samples[NUM_APP_CHANS], int num_out_chans);

#endif // _AUDIO_PROCESSING
