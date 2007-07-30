//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpodBufferRecorder.h"
#include "mp/MpMisc.h"
#include <os/OsTimer.h>
#include <os/OsNotification.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// DEFINES
#define FINAL_RECORDED_DATA_WRITE
#undef FINAL_RECORDED_DATA_WRITE

#ifdef FINAL_RECORDED_DATA_WRITE // [
#  include <os/OsFS.h>
#endif // FINAL_RECORDED_DATA_WRITE ]


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
// Default constructor
MpodBufferRecorder::MpodBufferRecorder(const UtlString& name,
                                       MpFrameTime bufferLength)
: MpOutputDeviceDriver(name)
, mBufferLengthMS(bufferLength)
, mBufferLength(0)
, mpBuffer(NULL)
, mBufferEnd(0)
, mpTickerTimer(NULL)
, mpTickerNotification(NULL)
{
}

MpodBufferRecorder::~MpodBufferRecorder()
{
   if (mpBuffer != NULL)
   {
      delete[] mpBuffer;
      mpBuffer = NULL;
   }

   if (mpTickerNotification != NULL)
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "Ticker notification is not cleared "
                                     "before driver destruction!");
      setTickerNotification(NULL);
   }

   if (isEnabled())
   {
      OsSysLog::add(FAC_MP, PRI_ERR, "Device is enabled while destroying device!");
      disableDevice();
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MpodBufferRecorder::enableDevice(unsigned samplesPerFrame, 
                                          unsigned samplesPerSec,
                                          MpFrameTime currentFrameTime)
{
   assert(!isEnabled());
   assert(mpBuffer == NULL);

   mSamplesPerFrame = samplesPerFrame;
   mSamplesPerSec = samplesPerSec;
   
   mBufferLength = mBufferLengthMS * mSamplesPerSec / 1000;

   mpBuffer = new MpAudioSample[mBufferLength];
   mBufferEnd = 0;

   mIsEnabled = TRUE;

   return OS_SUCCESS;
}

OsStatus MpodBufferRecorder::disableDevice()
{
   assert(isEnabled());

   if (mpBuffer != NULL)
   {
#ifdef FINAL_RECORDED_DATA_WRITE // [
      OsDateTime now;
      OsDateTime::getCurTime(now);
      char fileName[1024];

      snprintf(fileName,
               1024, "MpodBufferRecorder-%4d.%02d.%02d.%02d-%02d-%02d.%03d.s16",
               now.getYear(), now.getMonth(), now.getDay(),
               now.getHour(), now.getMinute(), now.getSecond(),
               now.getMicrosecond()/1000);

      OsFile::openAndWrite(fileName,
                           (char*)mpBuffer,
                           mBufferEnd*sizeof(MpAudioSample));
#endif // FINAL_RECORDED_DATA_WRITE ]

      delete[] mpBuffer;
      mpBuffer = NULL;
      mBufferLength = 0;
      mBufferEnd = 0;
   }

   mIsEnabled = FALSE;

   return OS_SUCCESS;
}

OsStatus MpodBufferRecorder::pushFrame(unsigned int numSamples,
                                       const MpAudioSample* samples,
                                       MpFrameTime frameTime)
{
   if (!isEnabled())
   {
      return OS_INVALID_STATE;
   }

   if (mBufferEnd > mBufferLength)
   {
      return OS_FAILED;
   }

   unsigned samplesToCopy = min(numSamples, mBufferLength-mBufferEnd);
   if (samples != NULL)
   {
      memcpy(mpBuffer+mBufferEnd, samples, sizeof(MpAudioSample)*samplesToCopy);
   } 
   else
   {
      memset(mpBuffer+mBufferEnd, 0, sizeof(MpAudioSample)*samplesToCopy);
   }

   mBufferEnd += samplesToCopy;

   return OS_SUCCESS;
}

OsStatus MpodBufferRecorder::setTickerNotification(OsNotification *pFrameTicker)
{
   if (mpTickerTimer != NULL)
   {
      mpTickerTimer->stop(TRUE);
      delete mpTickerTimer;
      mpTickerTimer = NULL;
   }

   mpTickerNotification = pFrameTicker;

   if (mpTickerNotification != NULL)
   {
      mpTickerTimer = new OsTimer(*mpTickerNotification);
      mpTickerTimer->periodicEvery(OsTime(10), OsTime(10));
   }

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean MpodBufferRecorder::isFrameTickerSupported() const
{
   return TRUE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
/* //////////////////////////// PRIVATE /////////////////////////////////// */
