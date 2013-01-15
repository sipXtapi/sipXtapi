//  
// Copyright (C) 2006-2013 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsWriteLock.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsProtectEvent.h"
#include "mp/MpRtpInputConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpPlcSilence.h"
#include "mp/MpRtpOutputConnection.h"
#include <mp/MprRtpDispatcher.h>
#include "mp/MprEncode.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MpMediaTask.h"
#include "mp/MprBridge.h"
#include "mp/MprFromStream.h"
#include "mp/MprFromFile.h"
#include "mp/MprFromMic.h"

#if defined (SPEEX_ECHO_CANCELATION)
#include "mp/MprSpeexEchoCancel.h"
#elif defined (SIPX_ECHO_CANCELATION)
#include "mp/MprEchoSuppress.h"
#endif

#include "mp/MprSpeexPreProcess.h"
#include "mp/MprMixer.h"
#include "mp/MprSplitter.h"
#include "mp/MprToSpkr.h"
#include "mp/MprToneGen.h"

#include "mp/NetInTask.h"
#include "mp/MprRecorder.h"
#include "mp/MpTypes.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlBoolean MpCallFlowGraph::sbSendInBandDTMF = true ;

#ifdef DOING_ECHO_CANCELATION  // [
UtlBoolean MpCallFlowGraph::sbEnableAEC = true ;
#else // DOING_ECHO_CANCELATION ][
UtlBoolean MpCallFlowGraph::sbEnableAEC = false ;
#endif // DOING_ECHO_CANCELATION ]

#ifdef HAVE_SPEEX // [
UtlBoolean MpCallFlowGraph::sbEnableAGC = false ;
UtlBoolean MpCallFlowGraph::sbEnableNoiseReduction = false ;
#else // HAVE_SPEEX ][
UtlBoolean MpCallFlowGraph::sbEnableAGC = false ;
UtlBoolean MpCallFlowGraph::sbEnableNoiseReduction = false ;
#endif // HAVE_SPEEX ]

#define INSERT_RECORDERS ///< splices recorders into flowgraph
// #undef INSERT_RECORDERS 

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpCallFlowGraph::MpCallFlowGraph(const char* locale,
                                 int samplesPerFrame, int samplesPerSec,
                                 OsMsgDispatcher *pNotifDispatcher)
: MpFlowGraphBase(samplesPerFrame, samplesPerSec, pNotifDispatcher)
, mConnTableLock(OsBSem::Q_FIFO, OsBSem::FULL)
, mToneIsGlobal(FALSE)
{
   UtlBoolean    boolRes;
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

   for (i=0; i<MAX_CONNECTIONS; i++)
   {
      mpInputConnections[i]  = NULL;
      mpOutputConnections[i] = NULL;
      mNumRtpStreams[i]      = -1;
      mpDecoders[i]          = NULL;
      mpEncoders[i]          = NULL;
      mpMcastMixer[i]        = NULL;
   }
   for (i=0; i<MAX_RECORDERS; i++) mpRecorders[i] = NULL;

   // create the resources and add them to the flow graph
   mpBridge           = new MprBridge("Bridge", MAX_CONNECTIONS + 1);
   mpFromFile         = new MprFromFile("FromFile");
#ifndef DISABLE_STREAM_PLAYER
   mpFromStream       = new MprFromStream("FromStream");
#endif
#ifndef DISABLE_LOCAL_AUDIO // [
   mpFromMic          = new MprFromMic("FromMic", MpMisc.pMicQ);
   mpMicSplitter      = new MprSplitter("MicSplitter", 2);
   mpBufferRecorder   = new MprRecorder("BufferRecorder");
#if defined (SPEEX_ECHO_CANCELATION)
   mpEchoCancel       = new MprSpeexEchoCancel("SpeexEchoCancel", MpMisc.pEchoQ);
#elif defined (SIPX_ECHO_CANCELATION)
   mpEchoCancel       = new MprEchoSuppress("SipxEchoCancel");
#endif
#ifdef HAVE_SPEEX // [
   mpSpeexPreProcess  = new MprSpeexPreprocess("SpeexPreProcess");
#endif // HAVE_SPEEX ]
#endif // DISABLE_LOCAL_AUDIO ]
   mpTFsMicMixer      = new MprMixer("TFsMicMixer", 2);
   mpTFsBridgeMixer   = new MprMixer("TFsBridgeMixer", 2);
   mpCallrecMixer     = new MprMixer("CallrecMixer", 2);
   mpToneFileSplitter = new MprSplitter("ToneFileSplitter", 2);
   mpMicCallrecSplitter = new MprSplitter("MicCallrecSplitter", 2);
   mpSpeakerCallrecSplitter = new MprSplitter("SpeakerCallrecSplitter", 2);
#ifndef DISABLE_LOCAL_AUDIO // [
   mpToSpkr           = new MprToSpkr("ToSpkr", MpMisc.pSpkQ, MpMisc.pEchoQ);
#endif // DISABLE_LOCAL_AUDIO ]
   mpToneGen          = new MprToneGen("ToneGen", locale);
#ifndef DISABLE_LOCAL_AUDIO
#ifdef SIPX_ECHO_CANCELATION /* [ */
   mpEchoCancel->setSpkrPal(mpToSpkr);
#endif /* SIPX_ECHO_CANCELATION ] */
#endif

   res = addResource(*mpBridge);            assert(res == OS_SUCCESS);
#ifndef DISABLE_STREAM_PLAYER
   res = addResource(*mpFromStream);        assert(res == OS_SUCCESS);
#endif
   res = addResource(*mpFromFile);          assert(res == OS_SUCCESS);
#ifndef DISABLE_LOCAL_AUDIO // [
   res = addResource(*mpFromMic);           assert(res == OS_SUCCESS);
   res = addResource(*mpMicSplitter);       assert(res == OS_SUCCESS);
   res = addResource(*mpBufferRecorder);    assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_CANCELATION // [
   res = addResource(*mpEchoCancel);        assert(res == OS_SUCCESS);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   res = addResource(*mpSpeexPreProcess);   assert(res == OS_SUCCESS);
#endif // HAVE_SPEEX ]
#endif // DISABLE_LOCAL_AUDIO ]
   res = addResource(*mpTFsMicMixer);       assert(res == OS_SUCCESS);
   res = addResource(*mpTFsBridgeMixer);    assert(res == OS_SUCCESS);
   res = addResource(*mpCallrecMixer);      assert(res == OS_SUCCESS);
   res = addResource(*mpToneFileSplitter);  assert(res == OS_SUCCESS);
   res = addResource(*mpMicCallrecSplitter);      assert(res == OS_SUCCESS);
   res = addResource(*mpSpeakerCallrecSplitter);  assert(res == OS_SUCCESS);
#ifndef DISABLE_LOCAL_AUDIO
   res = addResource(*mpToSpkr);            assert(res == OS_SUCCESS);
#endif
   res = addResource(*mpToneGen);           assert(res == OS_SUCCESS);

   // create the connections between the resources
   //////////////////////////////////////////////////////////////////////////


#ifndef DISABLE_LOCAL_AUDIO
   // connect: 
   // FromMic -> MicSplitter -> (EchoCancel) -> (PreProcessor) -> TFsMicMixer -> ..
   //                       \-> BufferRecorder
   //
   // .. -> TFsMicMixer -> MicCallrecSplitter -> Bridge
   
   MpResource *pLastResource; // Last resource in the chain
   pLastResource = mpFromMic;

   res = addLink(*pLastResource, 0, *mpMicSplitter, 0);
   assert(res == OS_SUCCESS);
   pLastResource = mpMicSplitter;

   // Buffer recorder will use port 1 of the splitter.
   res = addLink(*pLastResource, 1, *mpBufferRecorder, 0);
   assert(res == OS_SUCCESS);
   // We don't set last resource here on purpose, as the splitter
   // should be used as input again next.

#ifdef DOING_ECHO_CANCELATION /* [ */
   res = addLink(*pLastResource, 0, *mpEchoCancel, 0);
   assert(res == OS_SUCCESS);
   pLastResource = mpEchoCancel;
#endif /* DOING_ECHO_CANCELATION ] */

#ifdef HAVE_SPEEX // [
   res = addLink(*pLastResource, 0, *mpSpeexPreProcess, 0);
   assert(res == OS_SUCCESS);
   pLastResource = mpSpeexPreProcess;
#endif // HAVE_SPEEX ]

   res = addLink(*pLastResource, 0, *mpTFsMicMixer, 1);
   assert(res == OS_SUCCESS);

#endif /* DISABLE_LOCAL_AUDIO ] */

   res = addLink(*mpTFsMicMixer, 0, *mpMicCallrecSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpMicCallrecSplitter, 0, *mpBridge, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Bridge[0] -> [1]TFsBridgeMixer[0] -> [0]SpeakerCallrecSplitter[0] -> [0]ToSpkr

   res = addLink(*mpBridge, 0, *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);

   res = addLink(*mpTFsBridgeMixer, 0, *mpSpeakerCallrecSplitter, 0);
   assert(res == OS_SUCCESS);

#ifndef DISABLE_LOCAL_AUDIO
   res = addLink(*mpSpeakerCallrecSplitter, 0, *mpToSpkr, 0);
   assert(res == OS_SUCCESS);
#endif

   //////////////////////////////////////////////////////////////////////////
   // connect SpeakerCallrecSplitter -> CallrecMixer
   //             MicCallrecSplitter --/ 
   res = addLink(*mpMicCallrecSplitter, 1, *mpCallrecMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpSpeakerCallrecSplitter, 1, *mpCallrecMixer, 1);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect ToneGen -> FromStream -> FromFile -> Splitter -> TFsBridgeMixer
   //                                                       -> Mixer
   
#ifndef DISABLE_STREAM_PLAYER
   res = addLink(*mpToneGen, 0, *mpFromStream, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromStream, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);
#else
   res = addLink(*mpToneGen, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);
#endif

   res = addLink(*mpFromFile, 0, *mpToneFileSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 0, *mpTFsBridgeMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 1, *mpTFsMicMixer, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // enable the flow graph (and all of the resources within it)
   res = enable();
   assert(res == OS_SUCCESS);

   // disable the tone generator
   boolRes = mpToneGen->disable();      assert(boolRes);
   mToneGenDefocused = FALSE;

#ifndef DISABLE_STREAM_PLAYER
   // disable the from stream
   boolRes = mpFromStream->disable();   assert(boolRes);
#endif

   // disable mpCallrecMixer and splitters, they are enabled when we want to start recording
   boolRes = mpCallrecMixer->disable();           assert(boolRes);
   boolRes = mpMicCallrecSplitter->disable();     assert(boolRes);
   boolRes = mpSpeakerCallrecSplitter->disable(); assert(boolRes);


#ifndef DISABLE_LOCAL_AUDIO // [
   // disable the FromMic, EchoCancel, PreProcess and ToSpkr -- we cannot have focus yet...
   boolRes = mpFromMic->disable();                assert(boolRes);
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();             assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   boolRes = mpSpeexPreProcess->disable();        assert(boolRes);
#endif // HAVE_SPEEX ]
   boolRes = mpToSpkr->disable();                 assert(boolRes);
#endif // DISABLE_LOCAL_AUDIO ]

   // The next group of settings turns the mixers into 2-to-1 multiplexers.
   // When disabled, mixers default to passing input 0 to output, and with
   // this setup, when enabled, they pass input 1 to output.
   boolRes = mpTFsMicMixer->setWeight(0, 0);      assert(boolRes);
   boolRes = mpTFsMicMixer->setWeight(1, 1);      assert(boolRes);

   boolRes = mpTFsBridgeMixer->setWeight(0, 0);   assert(boolRes);
   boolRes = mpTFsBridgeMixer->setWeight(1, 1);   assert(boolRes);

   // set up weights for callrec mixer as they are zeroed in constructor
   // input 0 is from mic
   boolRes = mpCallrecMixer->setWeight(1, 0);   assert(boolRes);
   // input 1 is speaker
   boolRes = mpCallrecMixer->setWeight(1, 1);   assert(boolRes);

////////////////////////////////////////////////////////////////////////////
//
//  NOTE:  The following should be a runtime decision, not a compile time
//         decision... watch for it in an upcoming version... soon, I hope.
//  But, that needs to be coordinated with changes in ToSpkr and FromMic,
//  and some recorders should be skipped on Win/32.
//
//  A couple more bits of unfinished business:  The destructor should
//  clean up recorders and open record files, if any.
//
////////////////////////////////////////////////////////////////////////////
#ifdef INSERT_RECORDERS /* [ */
#ifndef DISABLE_LOCAL_AUDIO // [
   mpRecorders[RECORDER_MIC] = new MprRecorder("RecordMic");
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC]), *mpFromMic, 0);
   assert(res == OS_SUCCESS);
#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_MIC32K] = new MprRecorder("RecordMicH");
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC32K]), *mpFromMic, 1);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#ifdef DOING_ECHO_CANCELATION /* [ */
   mpRecorders[RECORDER_ECHO_OUT] = new MprRecorder("RecordEchoOut");
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_OUT]),
                                                    *mpEchoCancel, 0);
   assert(res == OS_SUCCESS);

#ifdef SIPX_ECHO_CANCELATION // [
   mpRecorders[RECORDER_ECHO_IN8] = new MprRecorder("RecordEchoIn8");
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN8]),
                                                    *mpEchoCancel, 1);
   assert(res == OS_SUCCESS);
#endif // SIPX_ECHO_CANCELATION ]

#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_ECHO_IN32] = new MprRecorder("RecordEchoIn32");
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN32]),
                                                    *mpEchoCancel, 2);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif /* DOING_ECHO_CANCELATION ] */

#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_SPKR32K] = new MprRecorder("RecordSpkrH");
   res = insertResourceAfter(*(mpRecorders[RECORDER_SPKR32K]), *mpToSpkr, 1);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif // ndef DISABLE_LOCAL_AUDIO ]
#endif /* INSERT_RECORDERS ] */

///////////////////////////////////////////////////////////////////////////////////
//  Moved the following recorder out of the ifdef, to make it permanently available,
//  for media server use.
//
   mpRecorders[RECORDER_SPKR] = new MprRecorder("RecordSpkr");
   res = insertResourceBefore(*(mpRecorders[RECORDER_SPKR]), *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);
///////////////////////////////////////////////////////////////////////////////////

   // Call Recording..  Always record calls.
   // create Call recorder and connect it to mpCallrecMixer
   mpRecorders[RECORDER_CALL] = new MprRecorder("RecordCall");
   res = addResource(*(mpRecorders[RECORDER_CALL]));
   assert(res == OS_SUCCESS);
   res = addLink(*mpCallrecMixer, 0, *(mpRecorders[RECORDER_CALL]), 0);
   assert(res == OS_SUCCESS);

   // ask the media processing task to manage the new flow graph
   pMediaTask = MpMediaTask::getMediaTask();
   res = pMediaTask->manageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   // start the flow graph
   res = pMediaTask->startFlowGraph(*this);
   assert(res == OS_SUCCESS);

   Zprintf("mpBridge=0x%X, " "mpConnection=0x%X, " "mpFromFile=0x%X\n" 
       "mpFromMic=0x%X, " "mpTFsMicMixer=0x%X" "mpTFsBridgeMixer=0x%X\n",
      (int) mpBridge, (int) mpConnections[0], (int) mpFromFile, 
      (int) mpFromMic, (int) mpTFsMicMixer, (int) mpTFsBridgeMixer);

#ifdef DOING_ECHO_CANCELATION /* [ */
   Zprintf("mpTFsMicMixer=0x%X, " "mpTFsBridgeMixer=0x%X\n"
      "mpToneFileSplitter=0x%X, " "mpToSpkr=0x%X, " "mpToneGen=0x%X\n"
      "mpEchoCancel=0x%X\n",
      (int) mpTFsMicMixer, (int) mpTFsBridgeMixer, (int) mpToneFileSplitter,
      (int) mpToSpkr, (int) mpToneGen, (int) mpEchoCancel);
#else /* DOING_ECHO_CANCELATION ] [ */
   Zprintf("mpTFsMicMixer=0x%X, " "mpTFsBridgeMixer=0x%X\n"
      "mpToneFileSplitter=0x%X, " "mpToSpkr=0x%X, " "mpToneGen=0x%X\n",
      (int) mpTFsMicMixer, (int) mpTFsBridgeMixer, (int) mpToneFileSplitter,
      (int) mpToSpkr, (int) mpToneGen, 0);
#endif /* DOING_ECHO_CANCELATION ] */
}

// Destructor
MpCallFlowGraph::~MpCallFlowGraph()
{
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

   // unmanage the flow graph
   pMediaTask = MpMediaTask::getMediaTask();
   res = pMediaTask->unmanageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   // wait until the flow graph is unmanaged.
   while (pMediaTask->isManagedFlowGraph(this))
      OsTask::delay(20);   // wait 20 msecs before checking again

   // $$$ I believe that we should just be able to delete the flow graph
   // $$$ at this point, but for now let's get rid of all the connections
   // $$$ and resources first.

   // remove the links between the resources
   res = removeLink(*mpBridge, 0);           assert(res == OS_SUCCESS);
#ifndef DISABLE_LOCAL_AUDIO
   res = removeLink(*mpFromMic, 0);          assert(res == OS_SUCCESS);
   res = removeLink(*mpMicSplitter, 0);      assert(res == OS_SUCCESS);
   // remove connection to buffer recorder.
   res = removeLink(*mpMicSplitter, 1);      assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_CANCELATION // [
   res = removeLink(*mpEchoCancel, 0);       assert(res == OS_SUCCESS);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   res = removeLink(*mpSpeexPreProcess, 0);  assert(res == OS_SUCCESS);
#endif // HAVE_SPEEX ]
#endif
   res = removeLink(*mpTFsMicMixer, 0);      assert(res == OS_SUCCESS);
#ifndef DISABLE_LOCAL_AUDIO
   res = removeLink(*mpTFsBridgeMixer, 0);   assert(res == OS_SUCCESS);
#endif
   res = removeLink(*mpToneGen, 0);          assert(res == OS_SUCCESS);
#ifndef DISABLE_STREAM_PLAYER
   res = removeLink(*mpFromStream, 0);       assert(res == OS_SUCCESS);
#endif
   res = removeLink(*mpFromFile, 0);         assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 1); assert(res == OS_SUCCESS);

   // remove links of call recording
   res = removeLink(*mpCallrecMixer, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpMicCallrecSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpMicCallrecSplitter, 1); assert(res == OS_SUCCESS);
#ifndef DISABLE_LOCAL_AUDIO
   res = removeLink(*mpSpeakerCallrecSplitter, 0); assert(res == OS_SUCCESS);
#endif
   res = removeLink(*mpSpeakerCallrecSplitter, 1); assert(res == OS_SUCCESS);

   // now remove (and destroy) the resources
#ifndef DISABLE_LOCAL_AUDIO // [
   res = removeResource(*mpFromMic);
   assert(res == OS_SUCCESS);
   delete mpFromMic;
   mpFromMic = NULL;

   res = removeResource(*mpMicSplitter);
   assert(res == OS_SUCCESS);
   delete mpMicSplitter;
   mpMicSplitter = NULL;

   res = removeResource(*mpBufferRecorder);
   assert(res == OS_SUCCESS);
   delete mpBufferRecorder;
   mpBufferRecorder = NULL;

#ifdef DOING_ECHO_CANCELATION // [
   res = removeResource(*mpEchoCancel);
   assert(res == OS_SUCCESS);
   delete mpEchoCancel;
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]

   res = removeResource(*mpTFsMicMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsMicMixer;

   res = removeResource(*mpTFsBridgeMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsBridgeMixer;

   res = removeResource(*mpToneFileSplitter);
   assert(res == OS_SUCCESS);
   delete mpToneFileSplitter;

#ifndef DISABLE_LOCAL_AUDIO // [
   res = removeResource(*mpToSpkr);
   assert(res == OS_SUCCESS);
   delete mpToSpkr;
#endif // DISABLE_LOCAL_AUDIO ]

   res = removeResource(*mpToneGen);
   assert(res == OS_SUCCESS);
   delete mpToneGen;

#ifndef DISABLE_STREAM_PLAYER
   res = removeResource(*mpFromStream);
   assert(res == OS_SUCCESS);
   delete mpFromStream;
#endif

   res = removeResource(*mpFromFile);
   assert(res == OS_SUCCESS);
   delete mpFromFile;

   // kill call recording resources
   res = removeResource(*mpMicCallrecSplitter);
   assert(res == OS_SUCCESS);
   delete mpMicCallrecSplitter;

   res = removeResource(*mpSpeakerCallrecSplitter);
   assert(res == OS_SUCCESS);
   delete mpSpeakerCallrecSplitter;

   res = removeResource(*mpCallrecMixer);
   assert(res == OS_SUCCESS);
   delete mpCallrecMixer;

   for (i=0; i<MAX_RECORDERS; i++) {
      if (NULL != mpRecorders[i]) {
         res = removeResource(*mpRecorders[i]);
         assert(res == OS_SUCCESS);
         delete mpRecorders[i];
         mpRecorders[i] = NULL;
      }
   }

   res = removeResource(*mpBridge);
   assert(res == OS_SUCCESS);
   delete mpBridge;
}

/* ============================ MANIPULATORS ============================== */

// Notification that this flow graph has just been granted the focus.
// Enable our microphone and speaker resources
OsStatus MpCallFlowGraph::gainFocus(void)
{
   UtlBoolean    boolRes;
#ifndef DISABLE_LOCAL_AUDIO // [

   // enable the FromMic, (EchoCancel), (PreProcessor), and ToSpkr -- we have focus
   boolRes = mpFromMic->enable();       assert(boolRes);

#ifdef DOING_ECHO_CANCELATION // [
   if (sbEnableAEC)
   {
      boolRes = mpEchoCancel->enable(); assert(boolRes);
   }
#endif // DOING_ECHO_CANCELATION ]

#ifdef HAVE_SPEEX // [
   if (sbEnableAGC || sbEnableNoiseReduction || sbEnableAEC) {
     boolRes = mpSpeexPreProcess->enable(); assert(boolRes);
     mpSpeexPreProcess->setAGC(sbEnableAGC);
     mpSpeexPreProcess->setNoiseReduction(sbEnableNoiseReduction);
   }
#endif // HAVE_SPEEX ]

   boolRes = mpToSpkr->enable();        assert(boolRes);

#endif // DISABLE_LOCAL_AUDIO ]

   // Re-enable the tone as it is now being heard
   if(mToneGenDefocused)
   {
      mpToneGen->enable();
      mToneGenDefocused = FALSE;
   }

#ifdef DOING_ECHO_CANCELATION // [
   if (!mpTFsMicMixer->isEnabled())  
   {
#ifndef DISABLE_LOCAL_AUDIO
      boolRes = mpEchoCancel->disable();
      assert(boolRes);
#endif
   }
#endif // DOING_ECHO_CANCELATION ]

   Nprintf("MpBFG::gainFocus(0x%X)\n", (int) this, 0,0,0,0,0);
   return OS_SUCCESS;
}

// Notification that this flow graph has just lost the focus.
// Disable our microphone and speaker resources
OsStatus MpCallFlowGraph::loseFocus(void)
{
   UtlBoolean    boolRes;

#ifndef DISABLE_LOCAL_AUDIO // [

   // disable the FromMic, (EchoCancel), (PreProcessor) and ToSpkr --
   // we no longer have the focus.

   boolRes = mpFromMic->disable();       assert(boolRes);
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();    assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   boolRes = mpSpeexPreProcess->disable(); assert(boolRes);
#endif // HAVE_SPEEX ]
   boolRes = mpToSpkr->disable();        assert(boolRes);

#endif //DISABLE_LOCAL_AUDIO ]

   // If the tone gen is not needed while we are out of focus disable it
   // as it is using resources while it is not being heard.
   if(mpToneGen->isEnabled() && 
      mpTFsBridgeMixer->isEnabled()) // Local tone only
      // Should also disable when remote tone and no connections
      // || (!mp???Mixer->isEnabled() && noConnections)) 
   {
      // osPrintf("Defocusing tone generator\n");
      mpToneGen->disable();
      mToneGenDefocused = TRUE;
   }

   Nprintf("MpBFG::loseFocus(0x%X)\n", (int) this, 0,0,0,0,0);
   return OS_SUCCESS;
}

OsStatus MpCallFlowGraph::postNotification(const MpResNotificationMsg& msg)
{
   // This is here as a hook to do it's own things in response to notifications
   // being sent up.
   OsStatus stat = OS_SUCCESS;

   switch(msg.getMsg())
   {
   case MpResNotificationMsg::MPRNM_FROMFILE_STOPPED:
      {
         // If a file just finished playing, there is some cleanup that needs to be 
         // done at the flowgraph level that we can queue up to do now.
         MpFlowGraphMsg cfgStopPlayMsg(MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY);
         OsMsgQ* pMsgQ = getMsgQ();
         assert(pMsgQ != NULL);
         // Send the cleanup message, use default timeout of infinity,
         // as it should get processed no prob, and is important to get done,
         // otherwise state would be inconsistent between fromfile resource and flowgraph.
         stat = pMsgQ->send(cfgStopPlayMsg);
      }
      break;

   case MpResNotificationMsg::MPRNM_RECORDER_FINISHED:
   case MpResNotificationMsg::MPRNM_RECORDER_STOPPED:
   case MpResNotificationMsg::MPRNM_RECORDER_ERROR:
      {
         // If a file just finished recording, let flowgraph to clean up.
         // This should be called from MediaTask context, thus it should be safe
         // to call handleOnMprRecorderDisabled() directly. Though this is A HACK!
         handleOnMprRecorderDisabled(msg.getOriginatingResourceName());
      }
      break;

   case MpResNotificationMsg::MPRNM_RECORDER_STARTED:
      {
         // If a file just started recording, let flowgraph to setup.
         // This should be called from MediaTask context, thus it should be safe
         // to call handleOnMprRecorderDisabled() directly. Though this is A HACK!
         handleOnMprRecorderEnabled(msg.getOriginatingResourceName());
      }
      break;

   default:
      break;
   }

   // Now, let the parent postNotification run and do it's work if the last operation
   // succeeded.
   if(stat == OS_SUCCESS)
   {
      stat = MpFlowGraphBase::postNotification(msg);
   }
   return stat;
}

// Start playing the indicated tone.
void MpCallFlowGraph::startTone(int toneId, int toneOptions)
{
   UtlBoolean boolRes;
   OsStatus  res;
   int i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_TONE, NULL,
                   NULL, NULL, toneOptions, 0);

   res = mpToneGen->startTone(toneId);        assert(res == OS_SUCCESS);

   res = postMessage(msg);

   // Only play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);
   }

   if (toneOptions & TONE_TO_NET) { // "mToneIsGlobal"
      // Notify outbound leg of all connections that we are playing a tone
      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpEncoders[i]) 
            MprEncode::startTone(mpEncoders[i]->getName(), *getMsgQ(), toneId);
      }
   }
   // mpToneGen->enable();
}

// Stop playing the tone (applies to all tone destinations).
void MpCallFlowGraph::stopTone(void)
{
   OsStatus  res;
   int i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_TONE, NULL,
                   NULL, NULL, 0, 0);

   // mpToneGen->disable();

   res = mpToneGen->stopTone();         assert(res == OS_SUCCESS);
   res = postMessage(msg);

   // Shut off the tone generator input to the Mixer resources
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);

   if (mToneIsGlobal) {
      // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
      // Notify outbound leg of all connections that we are playing a tone
      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpEncoders[i]) 
            MprEncode::stopTone(mpEncoders[i]->getName(), *getMsgQ());
      }
   }
}

int MpCallFlowGraph::closeRecorders(void)
{
   int ret = 0;
   int i;

   for (i=0; i<MAX_RECORDERS; i++) {
      if (mpRecorders[i]) {
         MprRecorder::stop(mpRecorders[i]->getName(), *getMsgQ());
         ret++;
      }
   }
   return ret;
}




////////////////////////////////////////////////////////////////////////////
//
// A simple method for starting the recorders on the hard phone
// INSERT_RECORDERS must be defined for this to work
// 
// bitmask of recorderBitmask is as follows:
//  0000 0000 0000 0000 0000 0000 0000 0000
//  ^^^^ ^^^^ ^^^^ ^^^^ ^^^^^^^^^ ^^^^ ^^^^
//                  |              ||| ||||  
//                  |              ||| |||+------- mic
//                  |              ||| ||+-------- echo out
//                  |              ||| |+--------- speaker
//                  |              ||| +---------- mic 32k sampling
//                  |              ||+------------ speaker 32k sampling
//                  |              |+------------- echo in 8K
//                  |              +-------------- echo in 32K
//                  | 
//                  +----------------------------- not used
//
////////////////////////////////////////////////////////////////////////////

#define MAXUNIXPATH 64

OsStatus MpCallFlowGraph::Record(int ms, 
      const char* playFilename, //if NULL, defaults to previous string
      const char* baseName,     //if NULL, defaults to previous string
      const char* endName,      //if NULL, defaults to previous string
      int recorderMask)
{
   static int  playIndex = 0;
   static int  saved_ms = 0;
   static char saved_playFilename[MAXUNIXPATH] = "";
   static char saved_baseName[MAXUNIXPATH] = "";
   static char saved_endName[MAXUNIXPATH] = "";
   OsStatus    res;

   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask();
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) {
         return pIF->Record(ms, playFilename, baseName, endName, recorderMask);
      }
      return OS_INVALID;
   }
   
   if (ms == 0)
      ms = saved_ms;
   
   if (playFilename == NULL)
      playFilename = saved_playFilename;
   
   if (baseName == NULL)
      baseName = saved_baseName;
   
   if (endName == NULL)
      endName = saved_endName;

   
   char *created_micNamePtr      = new char[MAXUNIXPATH]; 
   char *created_echoOutNamePtr  = new char[MAXUNIXPATH]; 
   char *created_spkrNamePtr     = new char[MAXUNIXPATH]; 
   char *created_mic32NamePtr    = new char[MAXUNIXPATH]; 
   char *created_spkr32NamePtr   = new char[MAXUNIXPATH]; 
   char *created_echoIn8NamePtr  = new char[MAXUNIXPATH]; 
   char *created_echoIn32NamePtr = new char[MAXUNIXPATH]; 
   
   if (recorderMask & 1)
      sprintf(created_micNamePtr,
                        "%sm%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_micNamePtr;
      created_micNamePtr = NULL;
   }

   if (recorderMask & 2)
      sprintf(created_echoOutNamePtr,
                        "%so%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoOutNamePtr;
      created_echoOutNamePtr = NULL;
   }

   if (recorderMask & 4)
      sprintf(created_spkrNamePtr,
                        "%ss%d_%s_8k.raw", baseName, playIndex,  endName);
   else {
      delete [] created_spkrNamePtr;
      created_spkrNamePtr = NULL;
   }

   if (recorderMask & 8)
      sprintf(created_mic32NamePtr,
                        "%sm%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_mic32NamePtr;
      created_mic32NamePtr = NULL;
   }

   if (recorderMask & 16)
      sprintf(created_spkr32NamePtr,
                        "%ss%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_spkr32NamePtr;
      created_spkr32NamePtr = NULL;
   }

   if (recorderMask & 32)
      sprintf(created_echoIn8NamePtr,
                        "%se%d_%s_8k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoIn8NamePtr;
      created_echoIn8NamePtr = NULL;
   }

   if (recorderMask & 64)
      sprintf(created_echoIn32NamePtr,
                        "%se%d_%s_32k.raw", baseName, playIndex, endName);
   else {
      delete [] created_echoIn32NamePtr;
      created_echoIn32NamePtr = NULL;
   }

   res = record(ms, -1, created_micNamePtr, created_echoOutNamePtr,
              created_spkrNamePtr, created_mic32NamePtr, created_spkr32NamePtr,
              created_echoIn8NamePtr, created_echoIn32NamePtr,
              playFilename, NULL, 0, 0);
   playIndex++;
   
   strcpy(saved_playFilename,playFilename);
   strcpy(saved_baseName,baseName);
   strcpy(saved_endName,endName);

   return res;
}

OsStatus MpCallFlowGraph::recordMic(int ms, MpAudioSample* pAudioBuf,
                                    int bufferSize)
{
   OsStatus stat = OS_FAILED;
#ifndef DISABLE_LOCAL_AUDIO // [
   stat = MprRecorder::startBuffer(mpBufferRecorder->getName(),
                                   *getMsgQ(), pAudioBuf, bufferSize, ms);
#endif // DISABLE_LOCAL_AUDIO ]
   return stat;
}

OsStatus MpCallFlowGraph::recordMic(int ms,
                                    int silenceLength,
                                    const char* fileName)
{
   OsStatus ret;
   ret = record(ms, silenceLength, fileName, NULL, NULL, NULL, NULL, NULL, NULL,
                NULL, NULL, 0, 0, MprRecorder::WAV_PCM_16);

   return ret;
}

OsStatus MpCallFlowGraph::record(int timeMS,
                                 int silenceLength,
                                 const char* micName /*= NULL*/,
                                 const char* echoOutName /*= NULL*/,
                                 const char* spkrName /*= NULL*/,
                                 const char* mic32Name /*= NULL*/,
                                 const char* spkr32Name /*= NULL*/, 
                                 const char* echoIn8Name /*= NULL*/,
                                 const char* echoIn32Name /*= NULL*/,
                                 const char* playName /*= NULL*/,
                                 const char* callName /*= NULL*/,
                                 int toneOptions /*= 0*/,
                                 int repeat /*= 0*/,
                                 MprRecorder::RecordFileFormat format)
{
   // Convert seconds to milliseconds.
   silenceLength = silenceLength*1000;

#ifndef DISABLE_LOCAL_AUDIO // [
   if (NULL != micName) {
      setupRecorder(RECORDER_MIC, micName,
                    timeMS, silenceLength, format);
   }
   if (NULL != echoOutName) {
      setupRecorder(RECORDER_ECHO_OUT, echoOutName,
                    timeMS, silenceLength, format);
   }
   if (NULL != echoIn8Name) {
      setupRecorder(RECORDER_ECHO_IN8, echoIn8Name,
                    timeMS, silenceLength, format);
   }
#ifdef HIGH_SAMPLERATE_AUDIO // [
   if (NULL != mic32Name) {
      setupRecorder(RECORDER_MIC32K, mic32Name,
                    timeMS, silenceLength, format);
   }
   if (NULL != spkr32Name) {
      setupRecorder(RECORDER_SPKR32K,spkr32Name,
                    timeMS, silenceLength, format);
   }
   if (NULL != echoIn32Name) {
      setupRecorder(RECORDER_ECHO_IN32,
                    echoIn32Name, timeMS, silenceLength, format);
   }
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif // DISABLE_LOCAL_AUDIO ]
   if (NULL != spkrName) {
      setupRecorder(RECORDER_SPKR, spkrName,
                    timeMS, silenceLength, format);
   }
   // set up call recorder
   if (NULL != callName) {
      setupRecorder(RECORDER_CALL, callName,
                    timeMS, silenceLength, format);
   }
   if (NULL != playName)
   {
      playFile(playName, repeat, toneOptions);
   }
   return OS_SUCCESS;
}

// Setup recording on one recorder
UtlBoolean MpCallFlowGraph::setupRecorder(RecorderChoice which,
                  const char* audioFileName, int time, 
                  int silenceLength,
                  MprRecorder::RecordFileFormat format)
{
   OsStatus res;

   if (NULL == mpRecorders[which]) {
      return FALSE;
   }


   // Turn on notifications from the recorder resource, as they'll be
   // needed to determine when record stops. Look into postNototification()
   // to see how this is handled by CallFlowgraph.
   MpResource::setNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = MprRecorder::startFile(mpRecorders[which]->getName(), *getMsgQ(),
                                audioFileName, format, time, silenceLength);

   return res == OS_SUCCESS;
}

// Start playing the indicated audio file.
OsStatus MpCallFlowGraph::playFile(const char* audioFileName, UtlBoolean repeat,
                                   int toneOptions, UtlBoolean autoStopOnFinish)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the file stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = MprFromFile::playFile(mpFromFile->getName(), *getMsgQ(),
                               getSamplesPerSec(), audioFileName, repeat,
                               autoStopOnFinish);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Start playing the indicated audio buffer.
OsStatus MpCallFlowGraph::playBuffer(char* audioBuf, 
                                     unsigned long bufSize,
                                     uint32_t inRate, 
                                     int type, 
                                     UtlBoolean repeat,
                                     int toneOptions, 
                                     OsProtectedEvent* event,
                                     UtlBoolean autoStopOnFinish)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the buffer stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = MprFromFile::playBuffer(mpFromFile->getName(), *getMsgQ(),
                                 audioBuf, bufSize, inRate, getSamplesPerSec(),
                                 type, repeat, event, autoStopOnFinish);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// pause the current audio file, if one exists.
OsStatus MpCallFlowGraph::pauseFile()
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpFromFile)
   {
      stat = MprFromFile::pauseFile(mpFromFile->getName(), *getMsgQ());
   }
   return stat;
}

// resume the current audio file, if one exists.
OsStatus MpCallFlowGraph::resumeFile()
{
   OsStatus stat = OS_NOT_FOUND;
   if(mpFromFile)
   {
      stat = MprFromFile::resumeFile(mpFromFile->getName(), *getMsgQ());
   }
   return stat;
}


// Stop playing the audio file.
void MpCallFlowGraph::stopFile(UtlBoolean closeFile)
{
   OsStatus  res;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY, NULL,
                   NULL, NULL, 0, 0);

   // mpFromFile->disable();

   res = MprFromFile::stopFile(mpFromFile->getName(), *getMsgQ());
   assert(res == OS_SUCCESS);
   res = postMessage(msg);

   // Shut off the tone generator/play sound input to the Mixer resource
   // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);
}

MpConnectionID MpCallFlowGraph::createConnection(int maxRtpStreams,
                                                 UtlBoolean isMcast)
{
   MpRtpInputConnection* pInputConnection;
   MprDecode**   pDecoders;
   MprMixer*     pMcastMixer;
   MpRtpOutputConnection* pOutputConnection;
   MprEncode*    pEncode;
   MpConnectionID found = -1;
   int            bridgePort;
   int            i;

   assert(isMcast || maxRtpStreams == 1);

   {
      // Acquire mConnTableLock lock to manipulate connections.
      OsLock connectionsLock(mConnTableLock);

      for (i=1; i<MAX_CONNECTIONS; i++) 
      {
         if (NULL == mpInputConnections[i] &&
             NULL == mpOutputConnections[i]) 
         {
            mpInputConnections[i] = (MpRtpInputConnection*) -1;
            mpOutputConnections[i] = (MpRtpOutputConnection*) -1;
            found = i;
            break;
         }
      }
   
      if (found < 0)
      {
         return -1;
      }

      // Reserve Bridge port for this connection.
      bridgePort = mpBridge->reserveFirstUnconnectedInput();
      if (bridgePort < 0) 
      {
         mpInputConnections[found] = NULL;
         mpOutputConnections[found] = NULL;
         return -1;
      }

      Zprintf("bridgePort = %d\n", bridgePort, 0,0,0,0,0);

      // Create resources for the input and output connections.
      UtlString inConnectionName("InputConnection-");
      UtlString decodeName("Decode-");
      UtlString mixerName("InputRtpMixer-");
      UtlString outConnectionName("OutputConnection-");
      UtlString encodeName("Encode-");
      char numBuf[20];
      sprintf(numBuf, "%d", found);
      inConnectionName.append(numBuf);
      decodeName.append(numBuf);
      decodeName.append("-");
      mixerName.append(numBuf);
      outConnectionName.append(numBuf);
      encodeName.append(numBuf);
      mpInputConnections[found] = 
         new MpRtpInputConnection(inConnectionName, found, NULL,
                                       maxRtpStreams,
                                       isMcast ? MprRtpDispatcher::MOST_RECENT_SSRC
                                               : MprRtpDispatcher::ADDRESS_AND_PORT);
      mNumRtpStreams[found] = maxRtpStreams;
      mIsMcastConnection[found] = isMcast;
      mpDecoders[found] = new MprDecode*[maxRtpStreams];
      int decodeNameLen = decodeName.length();
      for (i=0; i < maxRtpStreams; i++)
      {
         sprintf(numBuf, "%d", i);
         decodeName.append(numBuf);
         mpDecoders[found][i] = new MprDecode(decodeName);
         mpDecoders[found][i]->setConnectionId(found);
         mpDecoders[found][i]->setStreamId(i);
         MprDejitter *pDejitter = new MprDejitter();
         mpDecoders[found][i]->setMyDejitter(pDejitter, TRUE);
         decodeName.resize(decodeNameLen);
      }
      // Don't create mixer if there is only one RTP stream.
      if (maxRtpStreams > 1)
      {
         mpMcastMixer[found] = new MprMixer(mixerName, maxRtpStreams);
      }
      mpOutputConnections[found] = 
         new MpRtpOutputConnection(outConnectionName, found, NULL);
      mpEncoders[found] = new MprEncode(encodeName);
      mpEncoders[found]->setConnectionId(found);

      pInputConnection = mpInputConnections[found];
      pDecoders = mpDecoders[found];
      pMcastMixer = mpMcastMixer[found];
      pOutputConnection = mpOutputConnections[found];
      pEncode = mpEncoders[found];
   }

   // Enable resources
   pInputConnection->enable();
   // Don't enable decoders - they will be enabled automatically.
   if (pMcastMixer != NULL)
   {
      pMcastMixer->enable();
   }
   pOutputConnection->enable();
   pEncode->enable();

   // Add resources to the flowgraph
   OsStatus stat = addResource(*pInputConnection);
   assert(OS_SUCCESS == stat);
   for (i=0; i<maxRtpStreams; i++)
   {
      stat = addResource(*pDecoders[i]);
      assert(OS_SUCCESS == stat);
   }
   if (pMcastMixer != NULL)
   {
      stat = addResource(*pMcastMixer);
      assert(OS_SUCCESS == stat);
   }
   stat = addResource(*pOutputConnection);
   assert(OS_SUCCESS == stat);
   stat = addResource(*pEncode);
   assert(OS_SUCCESS == stat);

   // Link resources together.
   stat = addLink(*mpBridge, bridgePort, *pEncode, 0);
   assert(OS_SUCCESS == stat);
   stat = addLink(*pEncode, 0, *pOutputConnection, 0);
   assert(OS_SUCCESS == stat);
   if (maxRtpStreams == 1)
   {
      // Single RTP stream, skip mpMcastMixer.
      stat = addLink(*pInputConnection, 0, *pDecoders[0], 0);
      assert(OS_SUCCESS == stat);
      stat = addLink(*pDecoders[0], 0, *mpBridge, bridgePort);
      assert(OS_SUCCESS == stat);
   }
   else
   {
      // Multiple RTP streams. Insert mpMcastMixer before the bridge.
      for (i=0; i<maxRtpStreams; i++)
      {
         stat = addLink(*pInputConnection, i, *pDecoders[i], 0);
         assert(OS_SUCCESS == stat);
         stat = addLink(*pDecoders[i], 0, *pMcastMixer, i);
         assert(OS_SUCCESS == stat);
      }
      stat = addLink(*pMcastMixer, 0, *mpBridge, bridgePort);
      assert(OS_SUCCESS == stat);
   }

   return found;
}

OsStatus MpCallFlowGraph::deleteConnection(MpConnectionID connID)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   OsStatus       ret;

   assert((0 < connID) && (connID < MAX_CONNECTIONS));

   if ((NULL == mpInputConnections[connID]) || 
      (((MpRtpInputConnection*) -1) == mpInputConnections[connID]))
         return OS_INVALID_ARGUMENT;

   if ((NULL == mpOutputConnections[connID]) || 
      (((MpRtpOutputConnection*) -1) == mpOutputConnections[connID]))
         return OS_INVALID_ARGUMENT;

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION, NULL,
                      NULL, NULL, connID);

   if (isStarted()) {
      // postPone(1000); // testing...
      ret = postMessage(msg);
      // if (OS_SUCCESS == ret) {
         // synchronize();
      // }
      return ret;
   }

   handled = handleMessage(msg);
   if (handled)
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

// Start sending RTP and RTCP packets.
void MpCallFlowGraph::startSendRtp(OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket,
                                    MpConnectionID connID,
                                    SdpCodec* pPrimaryCodec,
                                    SdpCodec* pDtmfCodec)
{
   if(mpOutputConnections[connID])
   {
      // Set sockets to send to.
      mpOutputConnections[connID]->setSockets(rRtpSocket, rRtcpSocket);

      // Tell encoder which codecs to use (data codec and signaling codec)
      // and enable it.
      MprEncode::selectCodecs(mpEncoders[connID]->getName(), *getMsgQ(),
                              pPrimaryCodec, pDtmfCodec);
      MpResource::enable(mpEncoders[connID]->getName(), *getMsgQ());
      MpResource::enable(mpOutputConnections[connID]->getName(), *getMsgQ());
   }
}

// Stop sending RTP and RTCP packets.
void MpCallFlowGraph::stopSendRtp(MpConnectionID connID)
{
   if(mpOutputConnections[connID])
   {
       if(mpOutputConnections[connID])
       {
          // Release sockets, deselect codecs and disable encoder.
          mpOutputConnections[connID]->releaseSockets();
          MprEncode::deselectCodecs(mpEncoders[connID]->getName(), *getMsgQ());
          MpResource::disable(mpEncoders[connID]->getName(), *getMsgQ());
          MpResource::disable(mpOutputConnections[connID]->getName(), *getMsgQ());
       }
   }
}

// Start receiving RTP and RTCP packets.
void MpCallFlowGraph::startReceiveRtp(SdpCodec* pCodecs[],
                                      int numCodecs,
                                      OsSocket& rRtpSocket,
                                      OsSocket& rRtcpSocket,
                                      MpConnectionID connID)
{
   OsLock connLock(mConnTableLock);
   if(mpInputConnections[connID])
   {
      // Setup codecs for decoding
      if (numCodecs)
      {
         for (int i=0; i<mNumRtpStreams[connID]; i++)
         {
            MprDecode::selectCodecs(mpDecoders[connID][i]->getName(), *getMsgQ(),
                                    pCodecs, numCodecs);
         }
      }

      // Fence before calling asynchronous method setSockets().
      synchronize("MpCallFlowGraph::startReceiveRtp() connID=%d", connID);

      // Provide sockets
      mpInputConnections[connID]->setSockets(rRtpSocket, rRtcpSocket);

      // And finally enable decode resource, if this connection is unicast.
      // For multicast connections decoders will be enabled on the fly
      // to reduce CPU consumption.
      if (numCodecs && !mIsMcastConnection[connID])
      {
         for (int i=0; i<mNumRtpStreams[connID]; i++)
         {
            mpDecoders[connID][i]->enable();
         }
      }
   }
}

// Stop receiving RTP and RTCP packets.
void MpCallFlowGraph::stopReceiveRtp(MpConnectionID connID)
{
   OsLock connLock(mConnTableLock);
   if(mpInputConnections[connID])
   {
      mpInputConnections[connID]->releaseSockets();

      for (int i=0; i<mNumRtpStreams[connID]; i++)
      {
         MprDecode::deselectCodecs(mpDecoders[connID][i]->getName(), *getMsgQ());
         MprDecode::disable(mpDecoders[connID][i]->getName(), *getMsgQ());
      }
   }
}

// Enables/Disable the transmission of inband DTMF audio
UtlBoolean MpCallFlowGraph::setInbandDTMF(UtlBoolean bEnable)
{
   UtlBoolean bSave = sbSendInBandDTMF;
   sbSendInBandDTMF = bEnable;
   return bSave ;
}

// Set Echo Cancelation Mode.
UtlBoolean MpCallFlowGraph::setAECMode(FLOWGRAPH_AEC_MODE mode)
{
   // Selecting an unsupported mode will return false, but no error.
   // Not sure if this is the correct behavior.
   UtlBoolean bReturn = false;
   switch (mode) {

   case FLOWGRAPH_AEC_CANCEL:
   case FLOWGRAPH_AEC_CANCEL_AUTO:
#ifdef DOING_ECHO_CANCELATION // [
      sbEnableAEC = true;
      bReturn = true;
#endif // DOING_ECHO_CANCELATION ]
      break;

   case FLOWGRAPH_AEC_SUPPRESS:
#ifdef SIPX_ECHO_CANCELATION // [
      sbEnableAEC = true;
      bReturn = true;
#endif // SIPX_ECHO_CANCELATION ]
      break;

   case FLOWGRAPH_AEC_DISABLED:
      sbEnableAEC = false;
      bReturn = true;
      break;
   }

   // Should post a message to make changes effective, but we are static and
   // don't have an instance.
   return bReturn;
}

UtlBoolean MpCallFlowGraph::setAGC(UtlBoolean bEnable)
{
   UtlBoolean bReturn = FALSE;
#ifdef HAVE_SPEEX // [
   sbEnableAGC = bEnable;
   bReturn = true;
#endif // HAVE_SPEEX ]
   return bReturn;
}

UtlBoolean MpCallFlowGraph::setAudioNoiseReduction(UtlBoolean bEnable)
{
   UtlBoolean bReturn = FALSE;
#ifdef HAVE_SPEEX // [
   sbEnableNoiseReduction = bEnable;
   bReturn = true;
#endif // HAVE_SPEEX ]
   return bReturn;
}
 
/* ============================ ACCESSORS ================================= */

// Returns the type of this flow graph.
MpFlowGraphBase::FlowGraphType MpCallFlowGraph::getType()
{
   return MpFlowGraphBase::CALL_FLOWGRAPH;
}

/* ============================ INQUIRY =================================== */

// Returns TRUE if the indicated codec is supported.
UtlBoolean MpCallFlowGraph::isCodecSupported(SdpCodec& rCodec)
{
   // $$$ For now always return TRUE
   return TRUE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Handles an incoming message for the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleMessage(OsMsg& rMsg)
{
   UtlBoolean retCode;

   retCode = FALSE;

   if (rMsg.getMsgType() == OsMsg::STREAMING_MSG)
   {
#ifndef DISABLE_STREAM_PLAYER
      //
      // Handle Streaming Messages
      //
      MpStreamMsg* pMsg = (MpStreamMsg*) &rMsg ;
      switch (pMsg->getMsg())
      {
         case MpStreamMsg::STREAM_REALIZE_URL:
            retCode = handleStreamRealizeUrl(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_REALIZE_BUFFER:
            retCode = handleStreamRealizeBuffer(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PREFETCH:
            retCode = handleStreamPrefetch(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PLAY:
            retCode = handleStreamPlay(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_REWIND:
            retCode = handleStreamRewind(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_PAUSE:
            retCode = handleStreamPause(*pMsg) ;
            break;
         case MpStreamMsg::STREAM_STOP:
            retCode = handleStreamStop(*pMsg) ;
            break;         
         case MpStreamMsg::STREAM_DESTROY:
            retCode = handleStreamDestroy(*pMsg) ;
            break;
         default:         
            break;
      }
#else
      assert(false);
#endif
   }
   else
   {
      MpFlowGraphMsg* pMsg = (MpFlowGraphMsg*) &rMsg ;
      //
      // Handle Normal Flow Graph Messages
      //
      switch (pMsg->getMsg())
      {
      case MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION:
         retCode = handleRemoveConnection(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_START_PLAY:
         retCode = handleStartPlay(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_STOP_RECORD:
         // osPrintf("\n++++++ recording stopped\n");
         // retCode = handleStopRecord(rMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_START_TONE:
         retCode = handleStartTone(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY:
      case MpFlowGraphMsg::FLOWGRAPH_STOP_TONE:
         retCode = handleStopToneOrPlay();
         break;
      default:
         retCode = MpFlowGraphBase::handleMessage(*pMsg);
         break;
      }
   }

   return retCode;
}

UtlBoolean MpCallFlowGraph::handleRemoveConnection(MpFlowGraphMsg& rMsg)
{
   MpConnectionID connID = rMsg.getInt1();
   MpRtpInputConnection* pInputConnection;
   int           numRtpStreams;
   MprDecode**   pDecoders;
   MprMixer*     pMcastMixer;
   MpRtpOutputConnection* pOutputConnection;
   MprEncode*    pEncode;
   UtlBoolean    res;

   mConnTableLock.acquire();
   pInputConnection = mpInputConnections[connID];
   numRtpStreams = mNumRtpStreams[connID];
   pDecoders = mpDecoders[connID];
   pMcastMixer = mpMcastMixer[connID];
   pOutputConnection = mpOutputConnections[connID];
   pEncode = mpEncoders[connID];
   mpInputConnections[connID] = NULL;
   mNumRtpStreams[connID] = -1;
   mpDecoders[connID] = NULL;
   mpMcastMixer[connID] = NULL;
   mpOutputConnections[connID] = NULL;
   mpEncoders[connID] = NULL;
   mConnTableLock.release();

   assert(pInputConnection != NULL);
   assert(pDecoders != NULL);
   assert(pOutputConnection != NULL);
   assert(pEncode != NULL);

   res = handleRemoveResource(pInputConnection);
   assert(res);
   delete pInputConnection;

   for (int i=0; i<numRtpStreams; i++)
   {
      res = handleRemoveResource(pDecoders[i]);
      assert(res);
      delete pDecoders[i];
   }
   delete[] pDecoders;

   if (pMcastMixer != NULL)
   {
      res = handleRemoveResource(pInputConnection);
      assert(res);
      delete pInputConnection;
   }

   res = handleRemoveResource(pOutputConnection);
   assert(res);
   delete pOutputConnection;

   res = handleRemoveResource(pEncode);
   assert(res);
   delete pEncode;

   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartPlay(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   // Only play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);
   }

   if (toneOptions & TONE_TO_NET)
   {
      // Play the file audio through the Mixer resource,
      // shutting off the other audio input
      boolRes = mpTFsMicMixer->disable();   assert(boolRes);
   }
#ifndef DISABLE_LOCAL_AUDIO // [
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();  assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartTone(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   // boolRes = mpToneGen->enable();          assert(boolRes);

   // Only play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->disable();   assert(boolRes);
   }

   mToneIsGlobal = (toneOptions & TONE_TO_NET);
   if (mToneIsGlobal)
   {
      // Play the file audio through the Mixer resource,
      // shutting off the other audio input
      boolRes = mpTFsMicMixer->disable();   assert(boolRes);

      // We may be asked NOT to send inband DTMF to the remote
      // party.  This is accomplished by setting the tone gen
      // weight to zero.  At which point, the mixer will send
      // silence.
      if (!sbSendInBandDTMF)
      {
         boolRes = mpTFsMicMixer->setWeight(0, 1);      assert(boolRes);
      }
   }
#ifndef DISABLE_LOCAL_AUDIO // [
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();  assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStopToneOrPlay()
{
   UtlBoolean boolRes;

#ifdef DOING_ECHO_CANCELATION /* [ */
   MpMediaTask* pMediaTask;

   pMediaTask = MpMediaTask::getMediaTask();
#endif /* DOING_ECHO_CANCELATION ] */

   // Shut off the tone generator input to the Mixer resources
   boolRes = mpTFsBridgeMixer->enable();     assert(boolRes);
   boolRes = mpTFsMicMixer->enable();        assert(boolRes);

   // The weight of the tone gen / from file resource may have
   // be changed to zero if we were requested NOT to send inband
   // DTMF.  This code resets that weight.
   if (!sbSendInBandDTMF)
   {
      boolRes = mpTFsMicMixer->setWeight(1, 1); assert(boolRes);
   }
#ifndef DISABLE_LOCAL_AUDIO // [
#ifdef DOING_ECHO_CANCELATION // [
   if (sbEnableAEC && (this == pMediaTask->getFocus()))
   {
      boolRes = mpEchoCancel->enable();  assert(boolRes);
   }
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]
   return TRUE;
}

#ifdef DEBUG_POSTPONE /* [ */
   void MpCallFlowGraph::postPone(int ms)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SYNCHRONIZE,
         NULL, NULL, NULL, ms, 0);
      OsStatus  res;
   
      res = postMessage(msg);
      // osPrintf("MpCallFlowGraph::postPone(%d)\n", ms);
   }
#endif /* DEBUG_POSTPONE ] */

#ifndef DISABLE_STREAM_PLAYER

UtlBoolean MpCallFlowGraph::handleStreamRealizeUrl(MpStreamMsg& rMsg)
{ 
   int flags = rMsg.getInt1() ;
   Url* pUrl = (Url*) rMsg.getInt2() ;
   OsNotification* pNotifyHandle = (OsNotification*) rMsg.getPtr1() ;
   OsNotification* pNotifyEvents = (OsNotification*) rMsg.getPtr2() ;

   StreamHandle handle = NULL ;
   
   mpFromStream->realize(*pUrl, flags, handle, pNotifyEvents) ;
   delete pUrl ;

   pNotifyHandle->signal((intptr_t) handle) ;

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamRealizeBuffer(MpStreamMsg& rMsg)
{
   int flags = rMsg.getInt1() ;
   UtlString* pBuffer = (UtlString*) rMsg.getInt2() ;
   OsNotification* pNotifyHandle = (OsNotification*) rMsg.getPtr1() ;
   OsNotification* pNotifyEvents = (OsNotification*) rMsg.getPtr2() ;

   StreamHandle handle = NULL ;
   
   mpFromStream->realize(pBuffer, flags, handle, pNotifyEvents) ;

   pNotifyHandle->signal((intptr_t) handle) ;
   
   return TRUE ;
}



UtlBoolean MpCallFlowGraph::handleStreamPrefetch(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->prefetch(handle) ;

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamRewind(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->rewind(handle) ;

   return TRUE ;
}



UtlBoolean MpCallFlowGraph::handleStreamPlay(MpStreamMsg& rMsg)
{
   UtlBoolean boolRes ;
   StreamHandle handle = rMsg.getHandle() ;
   int iFlags ;

   if (mpFromStream->getFlags(handle, iFlags) == OS_SUCCESS)
   {
      // Should we play locally?
      if (iFlags & STREAM_SOUND_LOCAL)
      {
         boolRes = mpTFsBridgeMixer->disable();
         assert(boolRes);
      }
      else
      {
         boolRes = mpTFsBridgeMixer->enable();
         assert(boolRes);
      }

      // Should we play remotely?
      if (iFlags & STREAM_SOUND_REMOTE)
      {
         boolRes = mpTFsMicMixer->disable();
         assert(boolRes);
      }
      else
      {
         boolRes = mpTFsMicMixer->enable();
         assert(boolRes);
      }
   
      mpFromStream->play(handle) ;
      mpFromStream->enable() ;
   }

   return TRUE ;
}


UtlBoolean MpCallFlowGraph::handleStreamPause(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->pause(handle) ;

   return TRUE ;
}

UtlBoolean MpCallFlowGraph::handleStreamStop(MpStreamMsg& rMsg)
{
   UtlBoolean boolRes; 
   StreamHandle handle = rMsg.getHandle() ;
   int iFlags ;

   // now lets do the enabling of devices we disabled
   // earlier in the handleStreamPlay method
   mpFromStream->stop(handle) ;

   if (mpFromStream->getFlags(handle, iFlags) == OS_SUCCESS)
   {
      // did we play locally?
      if (iFlags & STREAM_SOUND_LOCAL)
      {
         boolRes = mpTFsBridgeMixer->enable();
         assert(boolRes);
      }
      
      // did we play remotely?
      if (iFlags & STREAM_SOUND_REMOTE)
      {
         boolRes = mpTFsMicMixer->enable();
         assert(boolRes);
      }      
   }   

   return TRUE ;
}

UtlBoolean MpCallFlowGraph::handleStreamDestroy(MpStreamMsg& rMsg)
{
   StreamHandle handle = rMsg.getHandle() ;

   mpFromStream->destroy(handle) ;

   return TRUE ;
}
#endif // DISABLE_STREAM_PLAYER

UtlBoolean MpCallFlowGraph::handleOnMprRecorderEnabled(const UtlString &resourceName)
{
   UtlBoolean boolRes; 
   
   // if this call recorder, also enable required resources
   if (  mpRecorders[RECORDER_CALL]
      && resourceName == mpRecorders[RECORDER_CALL]->getName())
   {
      boolRes = mpCallrecMixer->enable();
      assert(boolRes);

      boolRes = mpMicCallrecSplitter->enable();
      assert(boolRes);

      boolRes = mpSpeakerCallrecSplitter->enable();
      assert(boolRes);
   }

   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleOnMprRecorderDisabled(const UtlString &resourceName)
{
   UtlBoolean boolRes;

   if (  mpRecorders[RECORDER_CALL]
      && resourceName == mpRecorders[RECORDER_CALL]->getName())
   {
      // also disable mpCallrecMixer and splitters

      /* checks just in case this gets called when
      the resources have already been deleted
      */
      if (mpCallrecMixer)
      {
         boolRes = mpCallrecMixer->disable();
         assert(boolRes);
      }
      if (mpMicCallrecSplitter)
      {
         boolRes = mpMicCallrecSplitter->disable();
         assert(boolRes);
      }
      if (mpSpeakerCallrecSplitter)
      {
         boolRes = mpSpeakerCallrecSplitter->disable();
         assert(boolRes);
      }
   }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */



