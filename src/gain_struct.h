/*
 * gain_struct.h
 *
 *  Created on: 20.05.2015
 *      Author: steph
 */

#ifndef GAIN_STRUCT_H_
#define GAIN_STRUCT_H_

#include <stdint.h>

typedef struct gains {
    int32_t  crest_gain;
    int32_t normal_gain;
    int32_t crest_fac;
    int32_t tau;
} gains;


#endif /* GAIN_STRUCT_H_ */
