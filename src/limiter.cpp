#include "limiter.h"

Limiter::Limiter(float fs, float t_att, float t_hold, float t_rel)
{
    // set limiter parameters
    fs = fs;
    tAtt = t_att;
    tHold = t_hold;
    tRel = t_rel;
    fixedFS = float_to_fixed32(1/fs,31); // 1/48000

    // calculate buffer length
    attBufferLen = (int32_t)(fs*tAtt) & 0xffffffe;
    maxBufferLen = (int32_t)(fs*tAtt + tHold*fs + 0.5);
    halfBufferLen = attBufferLen/2;
    halfBufferFac = (int32_t)(1./(halfBufferLen+1)*pow(2.,31));

    // init state variables
    //thresholdGlobal = 0x7fffffff; // Limiter zu Beginn ausschalten
    //thresholdGlobal = float_to_fixed32(0.1,31);
    //makeUpGain      = 0x10000000; // zu Beginn keine Verstaerkung
    setThreshold(0.1);
    normal_oldGain = 0x7fffffff;
    crest_oldGain = 0x7fffffff;
    sum1 = 0;
    sum2 = 0;

    attPos = 0;
    maxPos = 0;
    halfBufferPos = 0;

    // calculate release coeffitiens
    normal_aRel = float_to_fixed32(exp(-1/(tRel*fs)),31);
    normal_bRel = 0x7fffffff-normal_aRel;

    // peak detektor variable
    oldPeak = 0;
    Peak = 0;

    // rms detector variable
    oldRms = 0;
    Rms = 0;

    crest2 = 0;
    tRelMax = float_to_fixed32(1.0,16);
    tAttMax = float_to_fixed32(0.08,16);
    tauAtt = 0;
    tauRel = 0;

    alpha = float_to_fixed32(exp(-1/(0.2*fs)),31);

    maxBuffer = new int32_t[maxBufferLen];
    halfBuffer1 = new int32_t[halfBufferLen];
    memset(halfBuffer1, 0, halfBufferLen*sizeof(int32_t));
    halfBuffer2 = new int32_t[halfBufferLen];
    memset(halfBuffer2, 0, halfBufferLen*sizeof(int32_t));
    attBufferRight = new int32_t[attBufferLen];
    attBufferLeft = new int32_t[attBufferLen];

    urn_init();
}

Limiter::~Limiter() {
    delete[] maxBuffer;
    delete[] halfBuffer1;
    delete[] halfBuffer2;
    delete[] attBufferRight;
    delete[] attBufferLeft;
}


gains Limiter::process_sample(int32_t input_sample[], int32_t output_sample[]) {
    // temporaere / Hilfsvariablen
    int i;
    int32_t actMax;
    int32_t gainReduceFactor;
    int32_t halfBufferOut;
    int32_t halfBufferIn;
    int64_t temp64;
    int32_t normal_gain;
    int32_t crest_gain;
    int32_t gainPlusMakeup;
    int32_t x2n;

    // 1. Store current sample in the lookahead time circular buffer, for later
    // use (and retrieve the value that falls out as the preliminary 'Output')
    output_sample[0] = attBufferLeft[attPos];
    output_sample[1] = attBufferRight[attPos];
    attBufferLeft[attPos]  = input_sample[0];
    attBufferRight[attPos] = input_sample[1];
    //attBufferLeft[attPos]  = urn()>>1;
    //attBufferRight[attPos] = urn()>>1;

    // 2. Find maximum within this circular buffer. This can also be implemented
    // efficiently with an hold algorithm.
    maxBuffer[maxPos] = max(abs(input_sample[0]), abs(input_sample[1]));
    actMax = 0;
    for (i=0; i<maxBufferLen; i++)
        actMax = max(actMax, maxBuffer[i]);

    // 4. Calculate necessary gain reduction factor (=1, if no gain reduction
    // takes place and <1 for any signal above 0 dBFS)
    // printf("%ld %ld\n", thresholdGlobal, actMax);
    // printf("actMax = %ld\n", actMax);
    gainReduceFactor = udiv32(thresholdGlobal,actMax);

    // 6. Add this gain reduction value to the first of the smaller circular
    // buffers to calculate the short time sum (add this value to a sum and
    // subtract the value that felt out of the circular buffer).
    //7. normalize the sum by dividing it by the length of the circular buffer
    // (-> / ('Lookahead Time' [samples] / 2))
    halfBufferOut = halfBuffer1[halfBufferPos];
    halfBufferIn = ((int64_t) gainReduceFactor * halfBufferFac) >> 31;
    //printf("HalfBufferFac = %ld", halfBufferIn);
    halfBuffer1[halfBufferPos] = halfBufferIn;
    sum1 = sum1 + halfBufferIn - halfBufferOut;

    // 8. repeat step 6 & 7 with this sum in the second circular buffer. The
    // reason for these steps is to transform dirac impulses to a triangle
    // (dirac -> rect -> triangle)
    halfBufferOut = halfBuffer2[halfBufferPos];
    halfBufferIn = ((int64_t) sum1 * halfBufferFac) >> 31;
    halfBuffer2[halfBufferPos] = halfBufferIn;
    sum2 = sum2 + halfBufferIn - halfBufferOut;

    normal_gain = sum2;
    crest_gain = sum2;

    // crest factor controlling
    x2n = max(output_sample[0], output_sample[1]);
    x2n = ((int64_t)x2n*x2n) >> 31;
    Peak = max(x2n,(((int64_t)alpha*oldPeak) >> 31) + (((int64_t)(0x7fffffff-alpha)*x2n) >> 31));
    Rms = (((int64_t)alpha*oldRms) >> 31) + (((int64_t)(0x7fffffff-alpha)*x2n) >> 31);
    oldPeak = Peak;
    oldRms = Rms;

    crest2 = udiv32(Rms,Peak); // 1/3

    //
    tauAtt = tAttMax << 1; // multiplay by two
    // Q16.16 * Q1.31 -> Q17.47; Q17.47 >> 16 -> Q1.31
    tauAtt = ((int64_t) tauAtt * crest2) >> 16; // sollte 2/3, ist es aber nicht

    tauRel = tRelMax << 1; // 1s * 2 = 2s
    tauRel = (((int64_t) tauRel * crest2) >> 16);// - tauAtt;

   crest_bRel = udiv32(fixedFS, tauRel);
   crest_aRel = 0x7fffffff-crest_bRel;



    // Release mittels IIR Filter ohne Crest
    if (normal_gain > normal_oldGain)
    {
        temp64  = (int64_t) normal_aRel * normal_oldGain;
        temp64 += (int64_t) normal_bRel * normal_gain;
        normal_gain = min(normal_gain, (int32_t) (temp64 >> 31));
    }
    normal_oldGain = normal_gain;



    // Release mittels IIR Filter mit Crest
    if (crest_gain > crest_oldGain)
    {
        temp64  = (int64_t) crest_aRel * crest_oldGain;
        temp64 += (int64_t) crest_bRel * crest_gain;
        crest_gain = min(crest_gain, (int32_t) (temp64 >> 31));
    }
    crest_oldGain = crest_gain;


    // Q4.28 * Q1.31 -> Q5.59; Q5.59 >> 31 -> Q4.28
    //gainPlusMakeup = ((int64_t) makeUpGain * normal_gain) >> 31;


    // 13. Apply this gain reduction to the preliminary 'Output' from step 1
    // Q1.31 * Q4.28 -> Q5.59; Q5.59 >> 28 -> Q1.31
    output_sample[0] = ((int64_t) output_sample[0] * crest_gain) >> 31;
    output_sample[1] = ((int64_t) output_sample[1] * crest_gain) >> 31;

    // Q1.31 * Q8.24 -> Q9.55; Q9.55 >> 24 -> Q1.31
    output_sample[0] = ((int64_t) output_sample[0] * makeUpGain) >> 23;
    output_sample[1] = ((int64_t) output_sample[1] * makeUpGain) >> 23;

    // Buffer pointer verschieben
    attPos++;
    if (attPos >= attBufferLen)
        attPos = 0;

    maxPos++;
    if (maxPos >= maxBufferLen)
        maxPos = 0;

    halfBufferPos++;
    if (halfBufferPos >= halfBufferLen)
        halfBufferPos = 0;

    return_gains.normal_gain = normal_gain;
    return_gains.crest_gain = crest_gain;
    return_gains.crest_fac = crest2;
    //return_gains.tau = crest_bRel;
    return_gains.tau = tauRel;
    return_gains.makeUpGain = makeUpGain;
    return return_gains;
}


int32_t Limiter::getThreshold(void)
{
    return thresholdGlobal;
}

void Limiter::setThreshold(float newThresh)
{
    float mugFloat = (1/newThresh)-0.0001;
    makeUpGain = float_to_fixed32(mugFloat,23);
    thresholdGlobal = float_to_fixed32(newThresh,31);

}
