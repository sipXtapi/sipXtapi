//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#ifdef __pingtel_on_posix__
#include <unistd.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsTask.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprRecorder.h"
#include "os/OsProtectEventMgr.h"

#ifndef ABS
#define ABS(x) (max((x), -(x)))
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
#define MIN_SPEECH_ENERGY_THRESHOLD 20000
#define MAX_SPEECH_ENERGY_THRESHOLD 70000

// Default the silence length to 5 secs
#define DEFAULT_SILENCE_LEN 5
#define DEFAULT_FRAMES_TO_RECORD 2000000000

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprRecorder::MprRecorder(const UtlString& rName,
                         int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 1, 1, 0, 1, samplesPerFrame, samplesPerSec),
   mTermKey(-1),
   mFileDescriptor(-1),
   mRecFormat(UNINITIALIZED_FORMAT),
   mTotalBytesWritten(0),
   mTotalSamplesWritten(0),
   mConsecutiveInactive(0),
   mSilenceLength(0),
   mPrevEnergy(MIN_SPEECH_ENERGY_THRESHOLD),
   mpEvent(NULL),
   mFramesToRecord(0),
   mStatus(RECORD_IDLE),
   mMutex(OsMutex::Q_FIFO)
{
}

// Destructor
MprRecorder::~MprRecorder()
{
   if (mFileDescriptor != -1)
   {
       // If when we get to the destructor and our file descriptor is not set to -1
       // then close it now.

       if (mRecFormat == WAV_PCM_16)
       {
          updateWaveHeaderLengths(mFileDescriptor);
       }
       close(mFileDescriptor);
       mFileDescriptor = -1;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprRecorder::setup(int file, RecordFileFormat recFormat, 
                            int timeMS, int silenceLength, OsEvent* event)
{
   MpFlowGraphMsg msg(SETUP, this, (void*) event, 
                      (void*)silenceLength, file, timeMS);

   mRecFormat = recFormat;
   if (isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::setup"
                    " -- attempt to setup while enabled!!");
      return OS_INVALID;
   }
   return postMessage(msg);
}

OsStatus MprRecorder::begin(void)
{
   if (isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::begin"
                    " -- attempt to begin while enabled!!");
      return OS_INVALID;
   }
   handleBegin();
   return OS_SUCCESS;
}

OsStatus MprRecorder::stop(void)
{
   MpFlowGraphMsg msg(STOP, this, NULL, NULL, 0, 0);

   if (!isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::stop"
         " -- attempt to stop while disabled!!\n");
      return OS_INVALID;
   }
   return postMessage(msg);
}

UtlBoolean MprRecorder::enable(void)
{
   if (mFileDescriptor > -1)
   {
      mStatus = RECORDING;
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::enable");
      UtlBoolean res = MpResource::enable();

      if (res && mpFlowGraph)
      {
         /* resource is enabled, we started recording, inform flowgraph
         so it can send media event to sipxtapi and enable some resources
         if this is call recorder */
         MpFlowGraphMsg msg(MpFlowGraphMsg::ON_MPRRECORDER_ENABLED,
                            NULL, (void*) this, NULL, mStatus);
         postMessage(msg);
      }

      return res;
   } else
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "MprRecorder::enable (No file designated!)");
   }
   return FALSE;
}

UtlBoolean MprRecorder::disable(Completion code)
{
   UtlBoolean res = FALSE;

   // Lock so that the file contents cannot be changed out
   // from under us while we are updating the file.
   OsLock lock(mMutex);

   if (mFileDescriptor > -1)
   {
      if (mRecFormat == WAV_PCM_16)
      {
         updateWaveHeaderLengths(mFileDescriptor);
      }
   }
   if (mStatus != code)
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "MprRecorder::disable to report progress "
                    "mState(%d) code(%d), mFileDescriptor(0x%08x)",
                    mStatus, code, mFileDescriptor);
      progressReport(code);
   }

   if (RECORDING == mStatus) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprRecorder::disable -- stopping recorder");
      progressReport(RECORD_STOPPED);
   } else 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG,
                    "MprRecorder::disable (not already recording)");
   }
   mConsecutiveInactive = 0;

   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::disable setting mpEvent (0x%08x) to NULL", 
                 (int)mpEvent);
   
   {
      if (mpEvent != NULL)
      {
         // event may be released, do not signal the event any more
         mpEvent = NULL;
      }

      if (mFileDescriptor > -1) 
      {
         close(mFileDescriptor);
         mFileDescriptor = -1;
      }
      res = (MpResource::disable() && (mFileDescriptor == -1));
   }

   if (res && mpFlowGraph)
   {
      /* resource is disabled, we stopped started recording, inform flowgraph
      so it can send media event to sipxtapi and disable some resources
      if this is call recorder */
      MpFlowGraphMsg msg(MpFlowGraphMsg::ON_MPRRECORDER_DISABLED,
         NULL, (void*) this, NULL, mStatus);
      postMessage(msg);
   }

   // TODO: New Resource Notification message to indicate recording
   //       complete should go here.

   return res;
}

UtlBoolean MprRecorder::closeRecorder()
{
   UtlBoolean ret = TRUE;

   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::closeRecorder entering"
                 " - mFileDescriptor=%d, mStatus=%d",
                 mFileDescriptor, mStatus);
   if (isEnabled())
   {
      ret = disable(RECORD_STOPPED);
   }

   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::closeRecorder leaving "
                 "- mFileDescriptor=%d, mStatus=%d",
                 mFileDescriptor, mStatus);
   return ret;
}

UtlBoolean MprRecorder::termDtmf(int currentToneKey)
{
	UtlBoolean res = FALSE;
	if (   (currentToneKey == 11) || (currentToneKey == 1)
       || (currentToneKey == 0)  || (currentToneKey == -1)) 
	{
	 // Only if it's the # or 1 or 0 key, we terminate the recording.
	 // This is a temp solution for Weck.
	 //  -  We may want to make this configurable in the near future.
	 //  -  Or we may want to modify the VXI engine to handle grammar
	 //     inside a record field.
		OsSysLog::add(FAC_MP, PRI_INFO, 
                    "MprRecorder::termDtmf entering "
                    "- key=%d, mFileDescriptor=%d, mStatus=%d",
                    currentToneKey, mFileDescriptor, mStatus);

		mTermKey = currentToneKey;
		if (mTermKey != -1)
      {
			res = closeRecorder();
      }
	}

	return res;
}

/* ============================ ACCESSORS ================================= */

void MprRecorder::getRecorderStats(double& nBytes,
                                   double& nSamples, Completion& status)
{
   OsLock lock(mMutex);

   nBytes = mTotalBytesWritten;
   nSamples = mTotalSamplesWritten;
   status = mStatus;
}

void MprRecorder::getRecorderStats(struct MprRecorderStats* p)
{
   OsLock lock(mMutex);

   p->mTotalBytesWritten = mTotalBytesWritten;
   p->mTotalSamplesWritten = mTotalSamplesWritten;
   p->mFinalStatus = mStatus;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
UtlBoolean MprRecorder::updateWaveHeaderLengths(int handle)
{
    UtlBoolean retCode = FALSE;

    // Lock so that the file contents cannot be changed out
    // from under us while we are updating the file.
    OsLock lock(mMutex);

    //find out how many bytes were written so far
    unsigned long length = lseek(handle,0,SEEK_END);
    
    //now go back to beginning
    lseek(handle,4,SEEK_SET);
    
    //and update the RIFF length
    unsigned long rifflength = length-8;
    write(handle, (char*)&rifflength,sizeof(length));

    //now seek to the data length
    lseek(handle,40,SEEK_SET);
    
    //this should be the length of just the data
    unsigned long datalength = length-44;
    write(handle, (char*)&datalength,sizeof(datalength));

    return retCode;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprRecorder::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
   int numBytes = 0;
   int numSamples = 0;
   MpAudioBufPtr in;
   const MpAudioSample* input;

   // Lock so that mFileDescriptor and file contents cannot be changed out
   // from under us while we are updating the file.
   OsLock lock(mMutex);

   // try to pass along first input
   if (inBufsSize > 0)
   {
      in.swap(inBufs[0]);
   }

   if (numOutputs() > 0) 
   {
      outBufs[0] = in;
   }

   if (!isEnabled) 
   {
      return TRUE;
   }

   if (mFileDescriptor < 0) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, "MprRecorder::doProcessFrame to disable recording because mFileDescriptor=%d, mStatus=%d",
            mFileDescriptor, mStatus);
      disable(RECORD_STOPPED); // just in case...
   }

   if (inBufsSize == 0) 
   {
      // no input buffers, indicate config error
      disable(INVALID_SETUP);
      return TRUE;
   }

   // maximum record time reached or final silence timeout.
   if ((0 >= mFramesToRecord--) || (mSilenceLength <= mConsecutiveInactive)) 
   {
      OsSysLog::add(FAC_MP, PRI_INFO,
         "MprRecorder::doProcessFrame to disable recording because"
         " mFramesToRecord=%d, mStatus=%d mSilenceLength=%d,"
         " mConsecutiveInactive=%d", mFramesToRecord,
         mStatus, mSilenceLength, mConsecutiveInactive);
      disable(RECORD_FINISHED);
   }
   else 
   {
      //now write the buffer out
      int bytesWritten = 0;

      // Write silence if no input
      if (!in.isValid()) 
      {
         in = MpMisc.mpFgSilence;
      }

      if (in->isActiveAudio()) 
      {
         mConsecutiveInactive = 0;
      } else 
      {
         mConsecutiveInactive++;
      }

      input = in->getSamplesPtr();
      numSamples = in->getSamplesNumber();
      numBytes = numSamples * sizeof(MpAudioSample);
      if (mFileDescriptor > -1)
      {
        bytesWritten = write(mFileDescriptor, (char *)input, numBytes);
      }
   
      if (bytesWritten != numBytes) 
      {
         disable(WRITE_ERROR);
      } else 
      {
         mTotalBytesWritten += numBytes;
         mTotalSamplesWritten += samplesPerFrame;
      }
   }
   return TRUE;
}

void MprRecorder::progressReport(Completion code)
{
   int ud;

   mStatus = code;

   {
      OsLock lock(mMutex); 

      if (NULL != mpEvent) 
      {
         mpEvent->getUserData(ud);
         OsSysLog::add(FAC_MP, PRI_DEBUG, 
                       "MprRecorder::progressReport(%d), "
                       "event=0x%x, &data=0x%X",
                       code, (int) mpEvent, ud);
         if (0 != ud)
         {
            MprRecorderStats *rs = (MprRecorderStats*) ud;
      
            //report current stats
            rs->mTotalBytesWritten   = mTotalBytesWritten;
            rs->mTotalSamplesWritten = mTotalSamplesWritten;
            rs->mFinalStatus = code;
            int sps = getSamplesPerSec();
            rs->mDuration = (1000 * mTotalSamplesWritten) / sps;
            rs->mDtmfTerm = mTermKey;
            OsSysLog::add(FAC_MP, PRI_DEBUG, 
                          "MprRecorder::progressReport "
                          "mTotalSamplesWritten(%d), "
                          "sample per second(%d) duration (%d)", 
                          (int)mTotalSamplesWritten, 
                          (int)sps, (int)rs->mDuration);

            //now trigger the event...and pass back our struct
            OsStatus ret;
            if (OS_SUCCESS != (ret = mpEvent->signal(code)))
            {
               OsSysLog::add(FAC_MP, PRI_WARNING, 
                             "MprRecorder::progressReport "
                             "signal failed, returned %d, try again", 
                             (int)ret);
               // the event was probably just reset, 
               // try again after waiting for 10 ms.
               OsTask::delay(10);
               int userdata;
               mpEvent->getUserData(userdata);
               OsSysLog::add(FAC_MP, PRI_WARNING, 
                             "user data - old (0x%08x), "
                             "new (0x%08x), event (0x%08x) ", 
                             ud, userdata, (int)mpEvent);

// Comment out the assert for production system
//	            assert(userdata == ud);
               if (userdata) 
               {
                  ret = mpEvent->signal(code);
                  OsSysLog::add(FAC_MP, PRI_WARNING, 
                                "MprRecorder::progressReport "
                                "signal again, returned %d ", 
                                (int)ret);
               }
            }
         }
         else
         {
               OsSysLog::add(FAC_MP, PRI_WARNING, 
                             "MprRecorder::progressReport "
                             "did not signal user data is 0 "
                             "for event 0x%08x", (int)mpEvent);
         }
      }
      else
      {
         OsSysLog::add(FAC_MP, PRI_DEBUG, 
                       "MprRecorder::progressReport(%d) "
                       "(No event)", code);
      }
   }
}

// Handle messages for this resource.
UtlBoolean MprRecorder::handleSetup(int file, int timeMS, 
                                    int silenceLength, 
                                    OsProtectedEvent* event)
{
   int iMsPerFrame = 
      (1000 * getSamplesPerFrame()) / getSamplesPerSec();

   if (isEnabled()) 
   {
      OsSysLog::add(FAC_MP, PRI_DEBUG, 
                    "MprRecorder::handleSetup -- "
                    "attempt to setup while enabled!!");
      return TRUE;
   }

   if (timeMS > 0) 
   {
      mFramesToRecord = timeMS / iMsPerFrame;
   } else 
   {
      mFramesToRecord = DEFAULT_FRAMES_TO_RECORD;
   }

   // If the silence length passed in is zero or less,
   // then use the default silence length.
   if (silenceLength <= 0)
   {
      silenceLength = DEFAULT_SILENCE_LEN;
   }

   // mSilenceLength is held in #frames
   // so we convert silenceLength to frames.
   mSilenceLength = 1000 * silenceLength / iMsPerFrame;

   OsSysLog::add(FAC_MP, PRI_INFO, "MprRecorder::handleSetup\n");

   {
      OsLock lock(mMutex); 
      mFileDescriptor = file;
      mpEvent = event;   
   }

   mStatus = RECORD_IDLE;
   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::handleSetup(%d, %d, 0x%X)... #frames=%d",
                 file, timeMS, (int) event, mFramesToRecord);
   return TRUE;
}

UtlBoolean MprRecorder::handleBegin()
{
   mTotalBytesWritten = 0;
   mTotalSamplesWritten = 0;
   progressReport(RECORDING);
   enable();
   return TRUE;
}

UtlBoolean MprRecorder::handleStop()
{
   disable(RECORD_STOPPED);
   return TRUE;
}

UtlBoolean MprRecorder::handleMessage(MpFlowGraphMsg& rMsg)
{
   OsSysLog::add(FAC_MP, PRI_DEBUG, 
                 "MprRecorder::handleMessage(%d)", rMsg.getMsg());
   switch (rMsg.getMsg())
   {
   case SETUP:
      {
         //params:  Int1 = file descriptor,
         //         Int2 = time to record in milliseconds,
         //         Ptr1 = OsEvent pointer
         int file = rMsg.getInt1();
         int iMSec = rMsg.getInt2();
         OsProtectedEvent* pEvent = (OsProtectedEvent*) rMsg.getPtr1();
         int silenceLength = (int) rMsg.getPtr2();
         return handleSetup(file, iMSec, silenceLength, pEvent);
      }
      break;

   case BEGIN:
      return handleBegin();
      break;

   case STOP:
      return handleStop();
      break;
   }
   return MpAudioResource::handleMessage(rMsg);
}

/* ============================ FUNCTIONS ================================= */


