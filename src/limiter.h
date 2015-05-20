#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fixed_point_functions.h"

class Limiter
{
 public:
  Limiter(float fs, float t_att, float t_hold, float t_rel);
  ~Limiter();

  int32_t process_sample(int32_t input_sample[], int32_t output_sample[]);

 private:
  float   fs;
  float   tAtt;
  float   tHold;
  float   tRel;
  int32_t fixedFS;


  int32_t attBufferLen;
  int32_t maxBufferLen;
  int32_t halfBufferLen;

  int32_t halfBufferFac;

  // Buffer erstellen
  int32_t *maxBuffer;
  int32_t *attBufferLeft;
  int32_t *attBufferRight;
  int32_t *halfBuffer1;
  int32_t *halfBuffer2;

  int32_t aRel;
  int32_t bRel;
  int32_t alpha;

  int32_t oldRms;
  int32_t Rms;
  int32_t Peak;
  int32_t oldPeak;

  int32_t crest2;
  int32_t tauAtt;
  int32_t tauRel;
  int32_t tRelMax;
  int32_t tAttMax;


  // Gedaechtnisvariablen
  int32_t oldGain;
  int32_t sum1;
  int32_t sum2;

  // Initialwerte fuer Bufferpointer
  int32_t attPos;
  int32_t maxPos;
  int32_t halfBufferPos;
  int32_t thresholdGlobal;
  int32_t makeUpGain;

};
