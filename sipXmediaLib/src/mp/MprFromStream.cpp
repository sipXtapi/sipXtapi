//  
// Copyright (C) 2006 SIPez LLC. 
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
#include <os/fstream>
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsDateTime.h"
#include "os/OsEvent.h"
#include "mp/MpBuf.h"
#include "mp/MprFromStream.h"
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileOpen.h"
#include "mp/MpAudioWaveFileRead.h"
#include "mp/mpau.h"
#include "mp/MpMisc.h"

#include "mp/MpStreamFeeder.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromStream::MprFromStream(const UtlString& rName,
                             int samplesPerFrame, 
                             int samplesPerSec)
: MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec)   
, mpStreamRenderer(NULL)
, mEventState(FeederStreamStoppedEvent)
, miStreamCount(1)
{
}

// Destructor
MprFromStream::~MprFromStream()
{
   destroyFeeders() ;
}

/* ============================ MANIPULATORS ============================== */


OsStatus MprFromStream::realize(Url urlSource,                   
                                int flags,
                                StreamHandle &handle,
                                OsNotification* pEvent)
{
   OsStatus status = OS_INVALID ;
   
   // Create and Initialize feeder
   MpStreamFeeder* pFeeder = new MpStreamFeeder(urlSource, flags) ;
   if (pEvent != NULL)
      pFeeder->setEventHandler(pEvent) ;

   // Realize
   status = pFeeder->realize() ;

   // If not successful, cleanup.
   if (status == OS_SUCCESS)
   {
       STREAMDESC* pDesc = new STREAMDESC ;
       pDesc->handle = (void*) miStreamCount++ ;
       pDesc->pFeeder = pFeeder ;
       mStreamList.push(pDesc) ;
       handle = pDesc->handle ;
   }
   else
   {
      MpFlowGraphMsg msg(SOURCE_DESTROY, this, pFeeder, 0, 0, 0);
      postMessage(msg);
   }     

   return status ;
}


OsStatus MprFromStream::realize(UtlString* pBuffer,
                                int flags,
                                StreamHandle &handle,
                                OsNotification* pEvent)
{
   OsStatus status = OS_INVALID ;
   
   // Create and Initialize Feeder
   MpStreamFeeder* pFeeder = new MpStreamFeeder(pBuffer, flags) ;
   if (pEvent != NULL)
      pFeeder->setEventHandler(pEvent) ;

   // Realize
   status = pFeeder->realize() ;

   // If not successful, cleanup.
   if (status == OS_SUCCESS)
   {
       STREAMDESC* pDesc = new STREAMDESC ;
       pDesc->handle = (void*) miStreamCount++ ;
       pDesc->pFeeder = pFeeder ;
       mStreamList.push(pDesc) ;
       handle = pDesc->handle ;
   }
   else
   {  
      MpFlowGraphMsg msg(SOURCE_DESTROY, this, pFeeder, 0, 0, 0);
      postMessage(msg);
   }     

   return status ;
}



OsStatus MprFromStream::prefetch(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      MpFlowGraphMsg msg(SOURCE_RENDER, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::prefetch handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */
   return status ;
}


OsStatus MprFromStream::play(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      if (mEventState != FeederStreamPausedEvent)
      {
         mEventState = FeederStreamStoppedEvent ;
      }

      MpFlowGraphMsg msg(SOURCE_PLAY, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::play handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return status ;
}


OsStatus MprFromStream::rewind(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      MpFlowGraphMsg msg(SOURCE_REWIND, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::rewind handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return status ;
}



OsStatus MprFromStream::pause(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;


   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      MpFlowGraphMsg msg(SOURCE_PAUSE, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::pause handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return status ;
}


OsStatus MprFromStream::stop(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      MpFlowGraphMsg msg(SOURCE_STOP, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::stop handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return status ;
}


OsStatus MprFromStream::destroy(StreamHandle handle)
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = removeStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      MpFlowGraphMsg msg(SOURCE_DESTROY, this, pFeeder, 0, 0, 0);
      status = postMessage(msg);
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::destroy handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */
      

   return status ;
}


OsStatus MprFromStream::getFlags(StreamHandle handle, int& flags) 
{
   OsStatus status = OS_INVALID ;

   MpStreamFeeder* pFeeder = getStreamFeeder(handle) ;
   if (pFeeder != NULL)
   {
      status = pFeeder->getFlags(flags) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::getFlags handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return status ;
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus MprFromStream::setStreamSource(MpStreamFeeder *pFeeder) 
{
   if ((pFeeder != mpStreamRenderer) && (mpStreamRenderer != NULL))
   {
      mpStreamRenderer->markPaused(TRUE) ;         
   }

   if (mpStreamRenderer != pFeeder)
   {
      mpStreamRenderer = pFeeder ;
      mbStreamChange = TRUE ;
   }

   return OS_SUCCESS ;
}


MpStreamFeeder* MprFromStream::getStreamSource()
{
   return mpStreamRenderer ;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprFromStream::doProcessFrame(MpBufPtr inBufs[],
                                         MpBufPtr outBufs[],
                                         int inBufsSize,
                                         int outBufsSize,
                                         UtlBoolean isEnabled,
                                         int samplesPerFrame,
                                         int samplesPerSecond)
{
   UtlBoolean bSentData = FALSE ;
   MpAudioBufPtr out;
   MpAudioSample *outbuf;
   int count;

   // Check params for sanity
   if (outBufsSize == 0)
       return FALSE;

   if (samplesPerFrame == 0)
       return FALSE;

   if (isEnabled) 
   {
      // Get new buffer
      out = MpMisc.RawAudioPool->getBuffer();
      if (!out.isValid())
          return FALSE;
      out->setSamplesNumber(samplesPerFrame);
      count = out->getSamplesNumber();

      if (mpStreamRenderer)
      {
         mbStreamChange = FALSE ;
         if (!mpStreamRenderer->isMarkedPaused())
         {
            out->setSpeechType(MpAudioBuf::MP_SPEECH_TONE);
            outbuf = out->getSamplesWritePtr();

            if (mpStreamRenderer->getFrame((unsigned short*) outbuf) == OS_SUCCESS)
            {
               bSentData = TRUE ;

               if (mEventState != FeederStreamPlayingEvent)
               {
#ifdef MP_STREAM_DEBUG /* [ */
                  osPrintf("MprFromStream: FeederEvent=FeederStreamPlayingEvent\n") ;
#endif /* MP_STREAM_DEBUG ] */

                  mEventState = FeederStreamPlayingEvent ;
                  mpStreamRenderer->fromStreamUpdate(FeederStreamPlayingEvent) ;
               }  
            }
            else
            {
               if (  (mEventState != FeederStreamStoppedEvent) && 
                     (mEventState != FeederStreamAbortedEvent))
               {
#ifdef MP_STREAM_DEBUG /* [ */
                  osPrintf("MprFromStream: FeederEvent=FeederStreamStoppedEvent\n") ;
#endif /* MP_STREAM_DEBUG ] */

                  mEventState = FeederStreamStoppedEvent ;
                  mpStreamRenderer->fromStreamUpdate(FeederStreamStoppedEvent) ;
               }
               disable();
            }
         }
         else
         {
            if (mEventState != FeederStreamPausedEvent)
            {
#ifdef MP_STREAM_DEBUG /* [ */
                  osPrintf("MprFromStream: FeederEvent=FeederStreamPausedEvent\n") ;
#endif /* MP_STREAM_DEBUG ] */

               mEventState = FeederStreamPausedEvent ;
               mpStreamRenderer->fromStreamUpdate(FeederStreamPausedEvent) ;
            }
         }       
      }

      if (!bSentData) 
      {
         outbuf = out->getSamplesWritePtr();
         memset(outbuf, 0, out->getSamplesNumber()*sizeof(MpAudioSample));
         out->setSpeechType(MpAudioBuf::MP_SPEECH_SILENT);      
      }
   }      
   else
   {
      // Resource is disabled. Passthrough input data
      out.swap(inBufs[0]);
   }

   // Push audio data downstream
   outBufs[0] = out;

   return (TRUE);
}


UtlBoolean MprFromStream::handleRender(MpStreamFeeder* pFeeder)
{
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {
      pFeeder->render() ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handleRender handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return TRUE ;
}


UtlBoolean MprFromStream::handlePlay(MpStreamFeeder* pFeeder)
{
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {
      // Undo paused state
      if (pFeeder->isMarkedPaused())
      {
         pFeeder->markPaused(FALSE) ;
      }
      else
      {
         pFeeder->render() ;
      }
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handlePlay handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   // Place into focus
   setStreamSource(pFeeder) ;

   return TRUE ;
}


UtlBoolean MprFromStream::handleRewind(MpStreamFeeder* pFeeder)
{
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {
      pFeeder->rewind() ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handleRewind handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return TRUE ;
}


UtlBoolean MprFromStream::handlePause(MpStreamFeeder* pFeeder)
{
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {
      pFeeder->markPaused(TRUE) ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handlePause handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return TRUE ;
}


UtlBoolean MprFromStream::handleStop(MpStreamFeeder* pFeeder)
{   
   UtlBoolean bActiveFeeder = (getStreamSource() == pFeeder) ;

   // Take the resource out of focus
   if (bActiveFeeder)
   {
      setStreamSource(NULL) ;      
   }

   // Tell the resource to stop when "stop" is called by the user, as 
   // opposed to being called by reaching the end of media, we fire 
   // FeederStreamAbortedEvent, instead of FeederStreamStoppedEvent.
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {
      pFeeder->stop() ;

      if ((mEventState != FeederStreamAbortedEvent) || mbStreamChange)
      {         
         if (bActiveFeeder)
         {
            mEventState = FeederStreamAbortedEvent ;            
            pFeeder->fromStreamUpdate(FeederStreamAbortedEvent) ;
         }
         else
         {
            pFeeder->fromStreamUpdate(FeederStreamAbortedEvent) ;
         }
      }
      else
      {
         osPrintf("** WARNING: MprFromStream::handleStop unexpected state.\n") ;
      }
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handleStop handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */

   return TRUE ;
}


UtlBoolean MprFromStream::handleDestroy(MpStreamFeeder* pFeeder)
{   
   assert(pFeeder != NULL) ;
   if (pFeeder != NULL)
   {            
      handleStop(pFeeder) ;
      pFeeder->fromStreamUpdate(FeederStreamDestroyedEvent) ;	  

      delete pFeeder ;
   }
#ifdef MP_STREAM_DEBUG /* [ */
   else
   {
	   osPrintf("** WARNING: MprFromStream::handleDestroy handed null feeder\n") ;
   }
#endif /* MP_STREAM_DEBUG ] */


   return TRUE ;
}

// Handle messages for this resource.
UtlBoolean MprFromStream::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean bHandled = FALSE ;

   switch (rMsg.getMsg()) 
   {
      case SOURCE_RENDER:
         bHandled = handleRender((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      case SOURCE_PLAY:
         bHandled = handlePlay((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      case SOURCE_REWIND:
         bHandled = handleRewind((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      case SOURCE_PAUSE:
         bHandled = handlePause((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      case SOURCE_STOP:
         bHandled = handleStop((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      case SOURCE_DESTROY:
         bHandled = handleDestroy((MpStreamFeeder*) rMsg.getPtr1()) ;
         break ;
      default:
         bHandled = MpAudioResource::handleMessage(rMsg);
         break;
   }
   return bHandled ;
}

// Get the stream feeder for the given handle
MpStreamFeeder* MprFromStream::getStreamFeeder(StreamHandle handle)
{
    MpStreamFeeder* pFeeder = NULL ;
    STREAMDESC* pDesc ;

    int iteratorHandle = mStreamList.getIteratorHandle() ;
    while ((pDesc = (STREAMDESC*) mStreamList.next(iteratorHandle)) != NULL)
    {
        if (pDesc->handle == handle)
        {
            pFeeder = pDesc->pFeeder ;
			assert(pFeeder != NULL) ;
            break ;
        }
    }
    mStreamList.releaseIteratorHandle(iteratorHandle) ;

    return pFeeder ;
}


// Removes the stream feeder from the stream list. The Stream feeder is 
// returned if found, someone else is responsible for deleting it.
MpStreamFeeder* MprFromStream::removeStreamFeeder(StreamHandle handle)
{
    MpStreamFeeder* pFeeder = NULL ;

#ifdef MP_STREAM_DEBUG /* [ */
osPrintf("MpStreamFeeder removing feeder: %d\n", handle) ;
#endif /* MP_STREAM_DEBUG ] */

    STREAMDESC* pDesc ;
    int iteratorHandle = mStreamList.getIteratorHandle() ;
    while ((pDesc = (STREAMDESC*) mStreamList.next(iteratorHandle)) != NULL)
    {
        if (pDesc->handle == handle)
        {
            pFeeder = pDesc->pFeeder ;
            mStreamList.remove(iteratorHandle) ;
            delete pDesc ;
            break ;
        }
    }

    mStreamList.releaseIteratorHandle(iteratorHandle) ;

#ifdef MP_STREAM_DEBUG /* [ */
   if (pFeeder == NULL)
   {
	   osPrintf("** WARNING: MprFromStream::removeStreamFeeder unable to find handle %08X\n", handle) ;
   }
#endif /* MP_STREAM_DEBUG ] */

    return pFeeder ;
}



//Stops, destroys, and frees all stream feeders
void MprFromStream::destroyFeeders()
{
    MpStreamFeeder* pFeeder = NULL ;

#ifdef MP_STREAM_DEBUG /* [ */
osPrintf("MpStreamFeeder destroy feeders\n") ;
#endif /* MP_STREAM_DEBUG ] */


    STREAMDESC* pDesc ;
    int iteratorHandle = mStreamList.getIteratorHandle() ;
    while ((pDesc = (STREAMDESC*) mStreamList.next(iteratorHandle)) != NULL)
    {
        pFeeder = pDesc->pFeeder ;
		assert(pFeeder != NULL) ;
        mStreamList.remove(iteratorHandle) ;
        delete pDesc ;
        handleDestroy(pFeeder) ;
    }
    mStreamList.releaseIteratorHandle(iteratorHandle) ;
}

     

/* ============================ FUNCTIONS ================================= */

