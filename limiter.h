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
  float   m_fs;
  float   m_tAtt;
  float   m_tHold;
  float   m_tRel;



  int32_t m_attBufferLen;
  int32_t m_maxBufferLen;
  int32_t m_halfBufferLen;

  int32_t m_halfBufferFac;

  // Buffer erstellen
  int32_t *m_maxBuffer;
  int32_t *m_attBufferLeft;
  int32_t *m_attBufferRight;
  int32_t *m_halfBuffer1;
  int32_t *m_halfBuffer2;

  int32_t m_aRel;
  int32_t m_bRel;
  int32_t m_alpha;

  int32_t m_oldRms;
  int32_t m_Rms;
  int32_t m_Peak;
  int32_t m_oldPeak;

  int32_t m_crest2;

  // Gedaechtnisvariablen
  int32_t m_oldGain;
  int32_t m_sum1;
  int32_t m_sum2;

  // Initialwerte fuer Bufferpointer
  int32_t m_attPos;
  int32_t m_maxPos;
  int32_t m_halfBufferPos;
  int32_t m_thresholdGlobal;
  int32_t m_makeUpGain;

};
