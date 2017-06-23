//  
// Copyright (C) 2008-2017 SIPez LLC. All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// SYSTEM INCLUDES
#include <cmath>
#include <os/OsIntTypes.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <mp/MpMisc.h>
#include <mp/MprNotchFilter.h>
#include <mp/MpFlowGraphBase.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS
const float MprNotchFilter::DEFAULT_TARGET_FREQ = 2175.0;
const float MprNotchFilter::DEFAULT_SAMPLING_RATE = 8000.0;
const float MprNotchFilter::DEFAULT_BW = 50.0;

//#define ENABLE_FILE_LOGGING
#ifdef ENABLE_FILE_LOGGING
static FILE *sgNotchFilterFile=NULL;
class OutFileInit
{
public:
   OutFileInit()
   {
      sgNotchFilterFile = fopen("/tmp/notchfilter.raw", "w");
   }

   ~OutFileInit()
   {
      fclose(sgNotchFilterFile);
   }
};

static OutFileInit sgOutFileInit;
#endif // ENABLE_FILE_LOGGING

/* //////////////////////////////// PUBLIC //////////////////////////////// */

MprNotchFilter::MprNotchFilter(const UtlString& rName)
: MpAudioResource(rName, 1, 1, 1, 1)
, mTargetFreq(DEFAULT_TARGET_FREQ)
, mSampleRate(DEFAULT_SAMPLING_RATE)
, mBW(DEFAULT_BW)
, mZ1(0.0)
, mZ2(0.0)
{
   initFilter();
}

// Destructor
MprNotchFilter::~MprNotchFilter()
{
}

/* =============================== CREATORS =============================== */

/* ============================= MANIPULATORS ============================= */

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */
UtlBoolean MprNotchFilter::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = TRUE;
   switch (rMsg.getMsg())
   {
   default:
      return MpResource::handleMessage(rMsg);
   }
   return msgHandled;
}


UtlBoolean MprNotchFilter::doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond)
{
   // We're disabled or have nothing to process.
   if ( outBufsSize == 0 || inBufsSize == 0 )
   {
      return TRUE;
   }
   if (!isEnabled)
   {
      outBufs[0] = inBufs[0];
      return TRUE;
   }

   // Handle non-default sample rates
   if(mSampleRate != samplesPerSecond)
   {
       mSampleRate = samplesPerSecond;
       initFilter();
   }

   return filterSamples(inBufs, inBufsSize, outBufs, outBufsSize, samplesPerFrame);
}

/* /////////////////////////////// PRIVATE //////////////////////////////// */
void MprNotchFilter::initFilter()
{
#if 0
   float cosine = cos(2.0*M_PI*(mTargetFreq/mSampleRate));
   float R = 1.0-3.0*(mBW/mSampleRate);
   float K = ((1.0-2.0*R*cosine+(R*R))/(2.0-2.0*cosine));
   mA0 = K;
   mA1 = -2.0*K*cosine;
   mA2 = K;
   mB1 = 2.0*R*cosine;
   mB2 = -(R*R);
   OsSysLog::add(FAC_MP, PRI_INFO, "MprNotchFilter::initFilter %f %f %f %f %f",
                 cosine, R,K,mA1,mB1);
#else
    mZ2 = 0.0;
    mZ1 = 0.0;
    float omega0T = mTargetFreq/(mSampleRate/2.0)*M_PI;
    float deltaT = mBW/(mSampleRate/2.0)*M_PI;
    mA2=(1.0-tan(deltaT/2.0))/(1.0+tan(deltaT/2.0));
    mA1=(1.0+mA2)*cos(omega0T);
    OsSysLog::add(FAC_MP, PRI_INFO, "MprNotchFilter::initFilter %f %f %f %f ",
                  omega0T, deltaT,mA1,mA2);
#endif
}

void MprNotchFilter::doFiltering(const MpAudioSample *x, MpAudioSample *y, int n)
{
#if 0
    static float x_2 = 0.0f;
    static float x_1 = 0.0f;
    static float y_1 = 0.0f;
    static float y_2 = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        y[i] = (MpAudioSample)((mA0 * x[i]) + (mA1 * x_1) + (mA2 * x_2) +
                               (mB1 * y_1) + (mB2 * y_2));

        x_2 = x_1;
        x_1 = x[i];
        y_2 = y_1;
        y_1 = y[i];
    }
#else
    float g = 0.0;
    float h = 0.0;
    float z0 = 0.0;
    for (int i = 0; i < n; i++)
    {
        g = x[i]+mZ2;
        h = mA1*mZ1 - mA2*g;
        z0 = x[i]+h;
        y[i] = (MpAudioSample)((g-h)/2.0);
        mZ2 = mZ1;
        mZ1 = z0;
    }
#endif
#ifdef ENABLE_FILE_LOGGING
    fwrite(y, 1, n*sizeof(short), sgNotchFilterFile);
#endif // ENABLE_FILE_LOGGING
}
UtlBoolean MprNotchFilter::filterSamples(MpBufPtr inBufs[], int inBufsSize,
                                         MpBufPtr outBufs[], int outBufsSize,
                                         int samplesPerFrame)
{
    MpAudioBufPtr pInBuf = inBufs[0];
    const MpAudioSample* inSamples = pInBuf->getSamplesPtr();
    MpAudioBufPtr pOutBuf = MpMisc.RawAudioPool->getBuffer();
    
    pOutBuf->setSpeechType(MP_SPEECH_UNKNOWN);
    pOutBuf->setSamplesNumber(samplesPerFrame);
    outBufs[0].swap(pOutBuf);
    outBufs[0].requestWrite();
    MpAudioBufPtr audio(outBufs[0]);
    doFiltering(inSamples, audio->getSamplesWritePtr(), samplesPerFrame);
    return TRUE;
}

/* ============================== FUNCTIONS =============================== */


