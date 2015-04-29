#include "Limiter.h"

Limiter::Limiter(float fs, float t_att, float t_hold, float t_rel)
{
  
  // set limiter parameters
  m_fs = fs;
  m_tAtt = t_att;
  m_tHold = t_hold;
  m_tRel = t_rel;

  // calculate buffer length
  m_attBufferLen = (int32_t)(m_fs*m_tAtt) & 0xffffffe;
  m_maxBufferLen = (int32_t)(m_fs*m_tAtt + m_tHold*m_fs + 0.5);
  m_halfBufferLen = m_attBufferLen/2;

  m_halfBufferFac = (int32_t)(1./(m_halfBufferLen+1)*pow(2.,31));
  
  // init state variables
  m_thresholdGlobal = 0x7fffffff; // Limiter zu Beginn ausschalten
  m_makeUpGain      = 0x10000000; // zu Beginn keine Verstaerkung
  m_oldGain = 0x7fffffff;
  m_sum1 = 0;
  m_sum2 = 0;
    
  m_attPos = 0;
  m_maxPos = 0;
  m_halfBufferPos = 0;

  // calculate release coeffitiens
  m_aRel = float_to_fixed32(exp(-1/(m_tRel*m_fs)),31);
  m_bRel = 0x7fffffff-m_aRel;


  // peak detektor variable
  int32_t m_oldPeak = 0;
  int32_t m_Peak = 0;

  // rms detector variable
  int32_t m_oldRms = 0;
  int32_t m_Rms = 0;

  int32_t m_crest2 = 0;
  int32_t m_tRelMax = float_to_fixed32(1.0,16);
  int32_t m_tAttMax = float_to_fixed32(0.08,16);


  m_alpha = float_to_fixed32(exp(-1/(0.2*m_fs)),31);


  m_maxBuffer = new int32_t[m_maxBufferLen];
  m_halfBuffer1 = new int32_t[m_halfBufferLen];
  m_halfBuffer2 = new int32_t[m_halfBufferLen];
  m_attBufferRight = new int32_t[m_attBufferLen];
  m_attBufferLeft = new int32_t[m_attBufferLen];
  
}

Limiter::~Limiter() {
 delete[] m_maxBuffer;
 delete[] m_halfBuffer1;
 delete[] m_halfBuffer2;
 delete[] m_attBufferRight;
 delete[] m_attBufferLeft;
}


int32_t Limiter::process_sample(int32_t input_sample[], int32_t output_sample[]) {

  // temporaere / Hilfsvariablen
    int i;
    int32_t actMax;
    int32_t gainReduceFactor;
    int32_t halfBufferOut;
    int32_t halfBufferIn;
    int64_t temp64;
    int32_t gain;
    int32_t gainPlusMakeup;
    int32_t x2n;

    // 1. Store current sample in the lookahead time circular buffer, for later
    // use (and retrieve the value that falls out as the preliminary 'Output')
    output_sample[0] = m_attBufferLeft[m_attPos];
    output_sample[1] = m_attBufferRight[m_attPos];
    m_attBufferLeft[m_attPos]  = input_sample[0];
    m_attBufferRight[m_attPos] = input_sample[1];

    // 2. Find maximum within this circular buffer. This can also be implemented
    // efficiently with an hold algorithm.
    m_maxBuffer[m_maxPos] = max(abs(input_sample[0]), abs(input_sample[1]));
    actMax = 0;
    for (i=0; i<m_maxBufferLen; i++)
        actMax = max(actMax, m_maxBuffer[i]);

    // 4. Calculate necessary gain reduction factor (=1, if no gain reduction
    // takes place and <1 for any signal above 0 dBFS)
    gainReduceFactor = udiv32(m_thresholdGlobal,actMax);

    // 6. Add this gain reduction value to the first of the smaller circular
    // buffers to calculate the short time sum (add this value to a sum and
    // subtract the value that felt out of the circular buffer).
    //7. normalize the sum by dividing it by the length of the circular buffer
    // (-> / ('Lookahead Time' [samples] / 2))
    halfBufferOut = m_halfBuffer1[m_halfBufferPos];
    halfBufferIn = ((int64_t) gainReduceFactor * m_halfBufferFac) >> 31;
    m_halfBuffer1[m_halfBufferPos] = halfBufferIn;
    m_sum1 = m_sum1 + halfBufferIn - halfBufferOut;
    
    // 8. repeat step 6 & 7 with this sum in the second circular buffer. The
    // reason for these steps is to transform dirac impulses to a triangle
    // (dirac -> rect -> triangle)
    halfBufferOut = m_halfBuffer2[m_halfBufferPos];
    halfBufferIn = ((int64_t) m_sum1 * m_halfBufferFac) >> 31;
    m_halfBuffer2[m_halfBufferPos] = halfBufferIn;
    m_sum2 = m_sum2 + halfBufferIn - halfBufferOut;

    gain = m_sum2;


    // crest factor controlling
    x2n = max(output_sample[0], output_sample[1]);
    x2n = ((int64_t)x2n*x2n) >> 31;
    m_Peak = max(x2n,((int64_t)m_alpha*m_oldPeak) >> 31 + ((int64_t)(0x7fffffff-m_alpha)*x2n) >> 31);
    m_Rms = ((int64_t)m_alpha*m_oldRms) >> 31 + ((int64_t)(0x7fffffff-m_alpha)*x2n) >> 31;
    m_oldPeak = m_Peak;
    m_oldRms = m_Rms;

    m_crest2 = udiv32(m_Rms,m_Peak);

    //
    m_tauAtt = m_tAttMax >> 1; // multiplay by two
    // Q16.16 * Q1.31 -> Q17.47; Q17.47 >> 16 -> Q1.31
    m_tauAtt = ((int64_t) m_tauAtt * m_crest2) >> 16;

    m_tauRel = m_tRelMax >> 1;
    m_tauRel = ((int64_t) m_tauRel * m_crest2) >> 16 - m_tauAtt;




    // todo: neues t_Rel ausrechnen und anwenden!!









    // Release mittels IIR Filter
    if (gain > m_oldGain)
    {
        temp64  = (int64_t) m_aRel * m_oldGain;
        temp64 += (int64_t) m_bRel * gain;
        gain = min(gain, (int32_t) (temp64 >> 31));
    }

    m_oldGain = gain;

    // Q1.31 * Q4.28 -> Q5.59; Q5.59 >> 31 -> Q4.28
    gainPlusMakeup = ((int64_t) m_makeUpGain * gain) >> 31;

    // 13. Apply this gain reduction to the preliminary 'Output' from step 1
    // Q1.31 * Q4.28 -> Q5.59; Q5.59 >> 28 -> Q1.31
    output_sample[0] = ((int64_t) output_sample[0] * gainPlusMakeup) >> 28;
    output_sample[1] = ((int64_t) output_sample[1] * gainPlusMakeup) >> 28;

    // Buffer pointer verschieben
    m_attPos++;
    if (m_attPos >= m_attBufferLen)
        m_attPos = 0;

    m_maxPos++;
    if (m_maxPos >= m_maxBufferLen)
        m_maxPos = 0;

    m_halfBufferPos++;
    if (m_halfBufferPos >= m_halfBufferLen)
        m_halfBufferPos = 0;

    return gainPlusMakeup;
  
}
