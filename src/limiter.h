#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fixed_point_functions.h"
#include "gain_struct.h"
#include "URN.h"

class Limiter
{
 public:
  Limiter(float fs, float t_att, float t_hold, float t_rel);
  ~Limiter();

  gains process_sample(int32_t input_sample[], int32_t output_sample[]);
  int32_t getThreshold(void);

  void setThreshold(float newThresh);


 private:
  float   fs;
  float   tAtt;
  float   tHold;
  float   tRel;
  int32_t fixedFS;
  gains return_gains;


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

  int32_t normal_aRel;
  int32_t normal_bRel;

  int32_t crest_aRel;
  int32_t crest_bRel;

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
  int32_t normal_oldGain;
  int32_t crest_oldGain;
  int32_t sum1;
  int32_t sum2;

  // Initialwerte fuer Bufferpointer
  int32_t attPos;
  int32_t maxPos;
  int32_t halfBufferPos;
  int32_t thresholdGlobal;
  int32_t makeUpGain;

};
