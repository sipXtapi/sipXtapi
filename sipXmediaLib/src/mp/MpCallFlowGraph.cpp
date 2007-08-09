//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
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

#if defined(WIN32) && !defined(WINCE) /* [ */
#   include <io.h>
#   include <fcntl.h>
#endif /* WIN32 && !WINCE ] */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsWriteLock.h"
#include "os/OsEvent.h"
#include "sdp/SdpCodec.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsProtectEvent.h"
#include "os/OsFS.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MpRtpOutputAudioConnection.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MpMediaTask.h"
#include "mp/MpStreamMsg.h"
#include "mp/MprBridge.h"
#include "mp/MprFromStream.h"
#include "mp/MprFromFile.h"
#include "mp/MprFromMic.h"
#include "mp/MprBufferRecorder.h"

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
#include "mp/MpAudioUtils.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define KHz8000
#undef  KHz32000
#ifdef KHz8000 /* [ */
const int MpCallFlowGraph::DEF_SAMPLES_PER_FRAME = 80;
const int MpCallFlowGraph::DEF_SAMPLES_PER_SEC   = 8000;
#endif /* KHz8000 ] */
#ifdef KHz32000 /* [ */
const int MpCallFlowGraph::DEF_SAMPLES_PER_FRAME = 320;
const int MpCallFlowGraph::DEF_SAMPLES_PER_SEC   = 32000;
#endif /* KHz32000 ] */

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

#define INSERT_RECORDERS // splices recorders into flowgraph
// #undef INSERT_RECORDERS 

#ifdef INSERT_RECORDERS /* [ */
static int WantRecorders = 1;
int wantRecorders(int flag) {
   int save = WantRecorders;
   WantRecorders = !flag;
   return save;
}
int wR() {return wantRecorders(0);}
int nwR() {return wantRecorders(1);}
#endif /* INSERT_RECORDERS ] */

#ifndef O_BINARY
#define O_BINARY 0      // O_BINARY is needed for WIN32 not for VxWorks or Linux
#endif
 
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpCallFlowGraph::MpCallFlowGraph(const char* locale,
                                 int samplesPerFrame, int samplesPerSec)
: MpFlowGraphBase(samplesPerFrame, samplesPerSec),
  mConnTableLock(OsBSem::Q_FIFO, OsBSem::FULL),
  mToneIsGlobal(FALSE),
#ifdef INCLUDE_RTCP /* [ */
  mulEventInterest(LOCAL_SSRC_COLLISION | REMOTE_SSRC_COLLISION),
#endif /* INCLUDE_RTCP ] */
  mPremiumSoundEnabled(TRUE)
{
   UtlBoolean    boolRes;
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

   for (i=0; i<MAX_CONNECTIONS; i++) mpInputConnections[i] = NULL;
   for (i=0; i<MAX_CONNECTIONS; i++) mpOutputConnections[i] = NULL;
   for (i=0; i<MAX_RECORDERS; i++) mpRecorders[i] = NULL;

   // create the resources and add them to the flow graph
   mpBridge           = new MprBridge("Bridge", MAX_CONNECTIONS + 1,
                                 samplesPerFrame, samplesPerSec);
   mpFromFile         = new MprFromFile("FromFile",
                                 samplesPerFrame, samplesPerSec);
   mpFromStream       = new MprFromStream("FromStream",
                                 samplesPerFrame, samplesPerSec);
#ifndef DISABLE_LOCAL_AUDIO // [
   mpFromMic          = new MprFromMic("FromMic",
                                 samplesPerFrame, samplesPerSec, MpMisc.pMicQ);
   mpMicSplitter      = new MprSplitter("MicSplitter", 2, 
                                 samplesPerFrame, samplesPerSec);
   mpBufferRecorder   = new MprBufferRecorder("BufferRecorder",
                                 samplesPerFrame, samplesPerSec);
#if defined (SPEEX_ECHO_CANCELATION)
   mpEchoCancel       = new MprSpeexEchoCancel("SpeexEchoCancel",
                                 samplesPerFrame, samplesPerSec);
#elif defined (SIPX_ECHO_CANCELATION)
   mpEchoCancel       = new MprEchoSuppress("SipxEchoCancel",
                                 samplesPerFrame, samplesPerSec);
#endif
#ifdef HAVE_SPEEX // [
   mpSpeexPreProcess  = new MprSpeexPreprocess("SpeexPreProcess",
                                 samplesPerFrame, samplesPerSec);
#endif // HAVE_SPEEX ]
#endif // DISABLE_LOCAL_AUDIO ]
   mpTFsMicMixer      = new MprMixer("TFsMicMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpTFsBridgeMixer   = new MprMixer("TFsBridgeMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpCallrecMixer     = new MprMixer("CallrecMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpToneFileSplitter = new MprSplitter("ToneFileSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpMicCallrecSplitter = new MprSplitter("MicCallrecSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpSpeakerCallrecSplitter = new MprSplitter("SpeakerCallrecSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
#ifndef DISABLE_LOCAL_AUDIO // [
   mpToSpkr           = new MprToSpkr("ToSpkr",
                                 samplesPerFrame, samplesPerSec,
                                 MpMisc.pSpkQ, MpMisc.pEchoQ);
#endif // DISABLE_LOCAL_AUDIO ]
   mpToneGen          = new MprToneGen("ToneGen",
                                 samplesPerFrame, samplesPerSec, 
                                 locale);
#ifndef DISABLE_LOCAL_AUDIO
#ifdef SIPX_ECHO_CANCELATION /* [ */
   mpEchoCancel->setSpkrPal(mpToSpkr);
#endif /* SIPX_ECHO_CANCELATION ] */
#endif

   res = addResource(*mpBridge);            assert(res == OS_SUCCESS);
   res = addResource(*mpFromStream);        assert(res == OS_SUCCESS);
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

#ifdef DOING_ECHO_CANCELATION /* [ */
   res = addLink(*pLastResource, 1, *mpSpeexPreProcess, 1);
   assert(res == OS_SUCCESS);
#endif /* DOING_ECHO_CANCELATION ] */

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
   
   res = addLink(*mpToneGen, 0, *mpFromStream, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromStream, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);

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

   // disable the from stream
   boolRes = mpFromStream->disable();   assert(boolRes);

   // disable the from file
   boolRes = mpFromFile->disable();     assert(boolRes);

   // disable mpCallrecMixer and splitters, they are enabled when we want to start recording
   boolRes = mpCallrecMixer->disable();     assert(boolRes);
   boolRes = mpMicCallrecSplitter->disable();         assert(boolRes);
   boolRes = mpSpeakerCallrecSplitter->disable();     assert(boolRes);

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

#ifdef INCLUDE_RTCP /* [ */
   // All the Media Resource seemed to have been started successfully.
   // Let's now create an RTCP Session so that we may be prepared to
   // report on the RTP connections that shall eventually be associated
   // with this flow graph

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Create an RTCP Session for this Flow Graph.  Pass the SSRC ID to be
   // used to identify our audio source uniquely within this RTP/RTCP Session.
   mpiRTCPSession = piRTCPControl->CreateSession(rand_timer32());

   // Subscribe for Events associated with this Session
   piRTCPControl->Advise((IRTCPNotify *)this);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

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
 if (WantRecorders) {
#ifndef DISABLE_LOCAL_AUDIO // [
   mpRecorders[RECORDER_MIC] = new MprRecorder("RecordMic",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC]), *mpFromMic, 0);
   assert(res == OS_SUCCESS);
#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_MIC32K] = new MprRecorder("RecordMicH",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_MIC32K]), *mpFromMic, 1);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#ifdef DOING_ECHO_CANCELATION /* [ */
   mpRecorders[RECORDER_ECHO_OUT] =
      new MprRecorder("RecordEchoOut", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_OUT]),
                                                    *mpEchoCancel, 0);
   assert(res == OS_SUCCESS);

   mpRecorders[RECORDER_ECHO_IN8] =
      new MprRecorder("RecordEchoIn8", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN8]),
                                                    *mpEchoCancel, 1);
   assert(res == OS_SUCCESS);

#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_ECHO_IN32] =
      new MprRecorder("RecordEchoIn32", samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_ECHO_IN32]),
                                                    *mpEchoCancel, 2);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif /* DOING_ECHO_CANCELATION ] */

#ifdef HIGH_SAMPLERATE_AUDIO // [
   mpRecorders[RECORDER_SPKR32K] = new MprRecorder("RecordSpkrH",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceAfter(*(mpRecorders[RECORDER_SPKR32K]), *mpToSpkr, 1);
   assert(res == OS_SUCCESS);
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif // ndef DISABLE_LOCAL_AUDIO ]
 }
#endif /* INSERT_RECORDERS ] */

///////////////////////////////////////////////////////////////////////////////////
//  Moved the following recorder out of the ifdef, to make it permanently available,
//  for media server use.
//
   mpRecorders[RECORDER_SPKR] = new MprRecorder("RecordSpkr",
                                 samplesPerFrame, samplesPerSec);
   res = insertResourceBefore(*(mpRecorders[RECORDER_SPKR]), *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);
///////////////////////////////////////////////////////////////////////////////////

   // Call Recording..  Always record calls.
   // create Call recorder and connect it to mpCallrecMixer
   mpRecorders[RECORDER_CALL] =
      new MprRecorder("RecordCall", samplesPerFrame, samplesPerSec);
   res = addResource(*(mpRecorders[RECORDER_CALL]));
   assert(res == OS_SUCCESS);
   res = addLink(*mpCallrecMixer, 0, *(mpRecorders[RECORDER_CALL]), 0);
   assert(res == OS_SUCCESS);

   // ask the media processing task to manage the new flow graph
   pMediaTask = MpMediaTask::getMediaTask(0);
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

#ifdef INCLUDE_RTCP /* [ */
   // Let's terminate the RTCP Session in preparation for call teardown

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Unsubscribe for Events associated with this Session
   piRTCPControl->Unadvise((IRTCPNotify *)this);

   // Terminate the RTCP Session
   piRTCPControl->TerminateSession(mpiRTCPSession);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

   // unmanage the flow graph
   pMediaTask = MpMediaTask::getMediaTask(0);
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
   res = removeLink(*mpFromStream, 0);       assert(res == OS_SUCCESS);
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

   res = removeResource(*mpFromStream);
   assert(res == OS_SUCCESS);
   delete mpFromStream;

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
#ifndef DISABLE_LOCAL_AUDIO // ]

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

#endif // DISABLE_LOCAL_AUDIO

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
         if (NULL != mpOutputConnections[i]) 
             mpOutputConnections[i]->startTone(toneId);
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
         if (NULL != mpOutputConnections[i]) 
             mpOutputConnections[i]->stopTone();
      }
   }
}

int MpCallFlowGraph::closeRecorders(void)
{
   int ret = 0;
   int i;

   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) return pIF->closeRecorders();
      return 0;
   }
   for (i=0; i<MAX_RECORDERS; i++) {
      if (mpRecorders[i]) {
         mpRecorders[i]->closeRecorder();
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
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
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

   res = record(ms, 999999, created_micNamePtr, created_echoOutNamePtr,
              created_spkrNamePtr, created_mic32NamePtr, created_spkr32NamePtr,
              created_echoIn8NamePtr, created_echoIn32NamePtr,
              playFilename, NULL, 0, 0, NULL);
   playIndex++;
   
   strcpy(saved_playFilename,playFilename);
   strcpy(saved_baseName,baseName);
   strcpy(saved_endName,endName);

   return res;
}


OsStatus MpCallFlowGraph::mediaRecord(int ms, 
                                   int silenceLength, 
                                   const char* fileName, 
                                   double& duration,
                                   int& dtmfTerm,
                                   MprRecorder::RecordFileFormat format,
                                   OsProtectedEvent* recordEvent)
{
  if (!recordEvent)   // behaves like ezRecord
    return ezRecord(ms, 
                    silenceLength, 
                    fileName, 
                    duration,
                    dtmfTerm,
                    format);

  // nonblocking version
   if (dtmfTerm)
   {
     for (int i=0; i<MAX_CONNECTIONS; i++) 
     {
         if (NULL != mpInputConnections[i]) 
         {
           mpInputConnections[i]->setDtmfTerm(mpRecorders[RECORDER_SPKR]);
         }
     }
   }

  return record(ms, silenceLength, NULL, NULL, fileName,
                 NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, recordEvent, format);

}

OsStatus MpCallFlowGraph::recordMic(UtlString* pAudioBuffer)
{
   OsStatus stat = OS_FAILED;
   stat = MprBufferRecorder::startRecording(
             mpBufferRecorder->getName(),
             *getMsgQ(), pAudioBuffer);
   return stat;
}

OsStatus MpCallFlowGraph::recordMic(int ms,
                                    int silenceLength,
                                    const char* fileName)
{
    OsStatus ret = OS_WAIT_TIMEOUT ;
    double duration ;

    MprRecorderStats rs;
    OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
    OsProtectedEvent* recordEvent = eventMgr->alloc();
    recordEvent->setUserData((intptr_t)&rs);

    int timeoutSecs = (ms/1000 + 1);
    OsTime maxEventTime(timeoutSecs, 0);

    record(ms, silenceLength, fileName, NULL, NULL,
                 NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 
                 NULL, MprRecorder::WAV_PCM_16);

    // Wait until the call sets the number of connections
    while(recordEvent->wait(0, maxEventTime) == OS_SUCCESS)
    {
        intptr_t info;
        recordEvent->getUserData(info);
        if (info)
        {
            rs = *((MprRecorderStats *)info);
            duration = rs.mDuration;
            if (rs.mFinalStatus != MprRecorder::RECORDING)
            {
                ret = OS_SUCCESS;
                break;
            }
            else
                recordEvent->reset();
        }
    }

    closeRecorders();
    // If the event has already been signaled, clean up
    if(OS_ALREADY_SIGNALED == recordEvent->signal(0))
    {
        eventMgr->release(recordEvent);
    }
    return ret;
}


OsStatus MpCallFlowGraph::ezRecord(int ms, 
                                   int silenceLength, 
                                   const char* fileName, 
                                   double& duration,
                                   int& dtmfTerm,
                                   MprRecorder::RecordFileFormat format)
{
   OsStatus ret = OS_WAIT_TIMEOUT;
   MprRecorderStats rs;
   OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
   OsProtectedEvent* recordEvent = eventMgr->alloc();
   recordEvent->setUserData((intptr_t)&rs);

   int timeoutSecs = (ms/1000 + 1);
   OsTime maxEventTime(timeoutSecs, 0);


   record(ms, silenceLength, NULL, NULL, fileName,
                 NULL, NULL, NULL, NULL, NULL, NULL, 0, 
                 0, recordEvent,format);

   if (dtmfTerm)
   {
     for (int i=0; i<MAX_CONNECTIONS; i++) 
     {
         if (NULL != mpInputConnections[i]) 
         {
           mpInputConnections[i]->setDtmfTerm(mpRecorders[RECORDER_SPKR]);
         }
     }
   }

   // Wait until the call sets the number of connections
   while(recordEvent->wait(0, maxEventTime) == OS_SUCCESS)
   {
      intptr_t info;
      recordEvent->getUserData(info);
      if (info)
      {
         rs = *((MprRecorderStats *)info);
         duration = rs.mDuration;
         dtmfTerm = rs.mDtmfTerm;
         if (rs.mFinalStatus != MprRecorder::RECORDING)
         {
           ret = OS_SUCCESS;
           break;
         }
         else
            recordEvent->reset();
      }

   }

  closeRecorders();
  // If the event has already been signaled, clean up
  if(OS_ALREADY_SIGNALED == recordEvent->signal(0))
  {
     eventMgr->release(recordEvent);
  }
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
                                 OsNotification* completion /*= NULL*/,
                                 MprRecorder::RecordFileFormat format)
{
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) {
         return pIF-> record(timeMS, silenceLength, micName, echoOutName, spkrName,
            mic32Name, spkr32Name, echoIn8Name, echoIn32Name,
            playName, callName, toneOptions, repeat, completion);
      }
      return OS_INVALID;
   }

#ifndef DISABLE_LOCAL_AUDIO // [
   if (NULL != micName) {
      setupRecorder(RECORDER_MIC, micName,
                    timeMS, silenceLength, completion, format);
   }
   if (NULL != echoOutName) {
      setupRecorder(RECORDER_ECHO_OUT, echoOutName,
                    timeMS, silenceLength, completion, format);
   }
   if (NULL != echoIn8Name) {
      setupRecorder(RECORDER_ECHO_IN8, echoIn8Name,
                    timeMS, silenceLength, completion, format);
   }
#ifdef HIGH_SAMPLERATE_AUDIO // [
   if (NULL != mic32Name) {
      setupRecorder(RECORDER_MIC32K, mic32Name,
                    timeMS, silenceLength, completion, format);
   }
   if (NULL != spkr32Name) {
      setupRecorder(RECORDER_SPKR32K,spkr32Name,
                    timeMS, silenceLength, completion, format);
   }
   if (NULL != echoIn32Name) {
      setupRecorder(RECORDER_ECHO_IN32,
                    echoIn32Name, timeMS, silenceLength, completion, format);
   }
#endif // HIGH_SAMPLERATE_AUDIO ]
#endif // DISABLE_LOCAL_AUDIO ]
   if (NULL != spkrName) {
      setupRecorder(RECORDER_SPKR, spkrName,
                    timeMS, silenceLength, completion, format);
   }
   // set up call recorder
   if (NULL != callName) {
      setupRecorder(RECORDER_CALL, callName,
                    timeMS, silenceLength, completion, format);
   }
   return startRecording(playName, repeat, toneOptions, completion);
}

OsStatus MpCallFlowGraph::startRecording(const char* audioFileName,
                  UtlBoolean repeat, int toneOptions, OsNotification* event)
{
   OsStatus  res = OS_SUCCESS;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_RECORD, NULL,
                   NULL, NULL, toneOptions, START_PLAY_NONE);

   if (NULL != audioFileName) {
      res = mpFromFile->playFile(audioFileName, repeat, event);
      if (res == OS_SUCCESS) {
         msg.setInt1(toneOptions);
         msg.setInt2(START_PLAY_FILE);
      }
   }

   res = postMessage(msg);
   return(res);
}


// Setup recording on one recorder
UtlBoolean MpCallFlowGraph::setupRecorder(RecorderChoice which,
                  const char* audioFileName, int timeMS, 
                  int silenceLength, OsNotification* event,
                  MprRecorder::RecordFileFormat format)
{
   int file = -1;
   OsStatus  res = OS_INVALID_ARGUMENT;

   if (NULL == mpRecorders[which]) {
      return FALSE;
   }

   if (NULL != audioFileName) {
        file = open(audioFileName, O_BINARY | O_CREAT | O_RDWR, 0600);
   }

   if (-1 < file) {
      if (format == MprRecorder::WAV_PCM_16)
      {
          writeWAVHeader(file);
      }

      res = mpRecorders[which]->setup(file, format, timeMS, silenceLength, (OsEvent*)event);
   }
   else
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR,
         "setupRecorder failed to open file %s, error code is %i",
         audioFileName, errno);
   }
   return (file != -1 && res == OS_SUCCESS);
}

// Start playing the indicated audio file.
OsStatus MpCallFlowGraph::playFile(const char* audioFileName, UtlBoolean repeat,
                                int toneOptions, OsNotification* event)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the file stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setAllNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = mpFromFile->playFile(audioFileName, repeat, event);

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
                                     int type, 
                                     UtlBoolean repeat,
                                     int toneOptions, 
                                     OsProtectedEvent* event)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the buffer stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setAllNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = mpFromFile->playBuffer(audioBuf, bufSize, type, repeat, event);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Stop playing the audio file.
void MpCallFlowGraph::stopFile(UtlBoolean closeFile)
{
   OsStatus  res;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY, NULL,
                   NULL, NULL, 0, 0);

   // mpFromFile->disable();

   res = mpFromFile->stopFile();       assert(res == OS_SUCCESS);
   res = postMessage(msg);

   // Shut off the tone generator/play sound input to the Mixer resource
   // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);
}

MpConnectionID MpCallFlowGraph::createConnection()
{
   int            i;
   MpConnectionID found = -1;
   int            bridgePort;
   MpRtpInputAudioConnection*  pInputConnection;
   MpRtpOutputAudioConnection*  pOutputConnection;

   mConnTableLock.acquire();
   for (i=1; i<MAX_CONNECTIONS; i++) 
   {
      if (NULL == mpInputConnections[i] &&
          NULL == mpOutputConnections[i]) 
      {
         mpInputConnections[i] = (MpRtpInputAudioConnection*) -1;
         mpOutputConnections[i] = (MpRtpOutputAudioConnection*) -1;
         found = i;
         i = MAX_CONNECTIONS;
      }
   }
   
   if (found < 0) {
      mConnTableLock.release();
      return -1;
   }
   UtlString inConnectionName("InputConnection-");
   UtlString outConnectionName("OutputConnection-");
   char numBuf[20];
   sprintf(numBuf, "%d", found);
   inConnectionName.append(numBuf);
   outConnectionName.append(numBuf);
   mpInputConnections[found] = 
       new MpRtpInputAudioConnection(inConnectionName,
                                     found, 
                                     getSamplesPerFrame(), 
                                     getSamplesPerSec());
   mpOutputConnections[found] = 
       new MpRtpOutputAudioConnection(outConnectionName,
                                      found, 
                                      getSamplesPerFrame(), 
                                      getSamplesPerSec());

   pInputConnection = mpInputConnections[found];
   pOutputConnection = mpOutputConnections[found];

   bridgePort = mpBridge->reserveFirstUnconnectedInput();

   if (bridgePort < 0) 
   {
      delete pInputConnection;
      delete pOutputConnection;
      mpInputConnections[found] = NULL;
      mpOutputConnections[found] = NULL;
      mConnTableLock.release();
      return -1;
   }

   mConnTableLock.release();

   Zprintf("bridgePort = %d\n", bridgePort, 0,0,0,0,0);

   pInputConnection->enable();
   pOutputConnection->enable();

   OsStatus stat = addResource(*pInputConnection);
   assert(OS_SUCCESS == stat);
   stat = addResource(*pOutputConnection);
   assert(OS_SUCCESS == stat);

   stat = addLink(*mpBridge, bridgePort, *pOutputConnection, 0);
   assert(OS_SUCCESS == stat);
   stat = addLink(*pInputConnection, 0, *mpBridge, bridgePort);
   assert(OS_SUCCESS == stat);

   return found;
}

OsStatus MpCallFlowGraph::deleteConnection(MpConnectionID connID)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   OsStatus       ret;

//   osPrintf("deleteConnection(%d)\n", connID);
   assert((0 < connID) && (connID < MAX_CONNECTIONS));

   if ((NULL == mpInputConnections[connID]) || 
      (((MpRtpInputAudioConnection*) -1) == mpInputConnections[connID]))
         return OS_INVALID_ARGUMENT;

   if ((NULL == mpOutputConnections[connID]) || 
      (((MpRtpOutputAudioConnection*) -1) == mpOutputConnections[connID]))
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

void MpCallFlowGraph::setPremiumSound(UtlBoolean enablePremiumSound)
{
   OsStatus  res;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_PREMIUM_SOUND,
      NULL, NULL, NULL, enablePremiumSound);

   res = postMessage(msg);
}

#define DEBUG_GIPS_PREMIUM_SOUND
#undef  DEBUG_GIPS_PREMIUM_SOUND

void MpCallFlowGraph::disablePremiumSound(void)
{
#ifdef _VXWORKS /* [ */
#ifdef DEBUG_GIPS_PREMIUM_SOUND /* [ */
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) pIF->disablePremiumSound();
      return;
   }
#endif /* DEBUG_GIPS_PREMIUM_SOUND ] */
#endif /* _VXWORKS ] */
   setPremiumSound(MpRtpInputAudioConnection::DisablePremiumSound);
}

void MpCallFlowGraph::enablePremiumSound(void)
{
#ifdef _VXWORKS /* [ */
#ifdef DEBUG_GIPS_PREMIUM_SOUND /* [ */
   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask(0);
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) pIF->enablePremiumSound();
      return;
   }
#endif /* DEBUG_GIPS_PREMIUM_SOUND ] */
#endif /* _VXWORKS ] */
   setPremiumSound(MpRtpInputAudioConnection::EnablePremiumSound);
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
       MpRtpOutputAudioConnection::startSendRtp(*(getMsgQ()),
                                                mpOutputConnections[connID]->getName(),
                                                rRtpSocket, 
                                                rRtcpSocket,
                                                pPrimaryCodec, 
                                                pDtmfCodec);
   }
}

// (old style call...)
void MpCallFlowGraph::startSendRtp(SdpCodec& rPrimaryCodec,
                                    OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket,
                                    MpConnectionID connID)
{
   startSendRtp(rRtpSocket, rRtcpSocket, connID, &rPrimaryCodec, NULL);
}

// Stop sending RTP and RTCP packets.
void MpCallFlowGraph::stopSendRtp(MpConnectionID connID)
{
   // postPone(40); // testing...
   if(mpOutputConnections[connID])
   {
       if(mpOutputConnections[connID])
       {
           MpRtpOutputAudioConnection::stopSendRtp(*(getMsgQ()),
                                                mpOutputConnections[connID]->getName());
       }
   }
}

// Start receiving RTP and RTCP packets.
#ifdef OLD_WAY /* [ */
void MpCallFlowGraph::startReceiveRtp(SdpCodec& rCodec,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket,
                                       MpConnectionID connID)
{
   SdpCodec* pCodecs[1];

   pCodecs[0] = &rCodec;
   startReceiveRtp(pCodecs, 1, rRtpSocket, rRtcpSocket, connID);
}
#endif /* OLD_WAY ] */

void MpCallFlowGraph::startReceiveRtp(SdpCodec* pCodecs[],
                                       int numCodecs,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket,
                                       MpConnectionID connID)
{
    if(mpInputConnections[connID])
    {
        MpRtpInputAudioConnection::startReceiveRtp(*(getMsgQ()),
                                mpInputConnections[connID]->getName(),
                                pCodecs, 
                                numCodecs, 
                                rRtpSocket, 
                                rRtcpSocket);
    }
}

// Stop receiving RTP and RTCP packets.
void MpCallFlowGraph::stopReceiveRtp(MpConnectionID connID)
{
    if(mpInputConnections[connID])
    {
        MpRtpInputAudioConnection::stopReceiveRtp(*(getMsgQ()),
                                mpInputConnections[connID]->getName());
    }
}

OsStatus MpCallFlowGraph::addToneListener(OsNotification* pNotify,
                                             MpConnectionID connectionId)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY,
      NULL, pNotify, NULL, connectionId);

   return postMessage(msg);
}

OsStatus MpCallFlowGraph::removeToneListener(MpConnectionID connectionId)
{
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY,
      NULL, NULL, NULL, connectionId);

   return postMessage(msg);
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
   // Selecting an unsupporetd mode will return false, but no error.
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

 
#ifdef INCLUDE_RTCP /* [ */

/* ======================== CALLBACK METHODS ============================= */

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *              
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *              .
 *               
 * Usage Notes: 
 *
 */
void MpCallFlowGraph::LocalSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                         IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// We have a collision with our local SSRC.  We will remedy this by
// generating a new SSRC
    mpiRTCPSession->ReassignSSRC(rand_timer32(),
                         (unsigned char *)"LOCAL SSRC COLLISION");

// We must inform all connections associated with this session to change their
// SSRC
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++) 
    {
      if (mpOutputConnections[iConnection]->getRTCPConnection()) 
      {
//       Set the new SSRC
         mpOutputConnections[iConnection]->
                          reassignSSRC((int)mpiRTCPSession->GetSSRC());
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();

   return;
}


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *              
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *              .
 *               
 * Usage Notes: 
 *
 */
void MpCallFlowGraph::RemoteSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                          IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// According to standards, we are supposed to ignore remote sites that
// have colliding SSRC IDS.
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++) 
    {
      if (mpInputConnections[iConnection] &&
          mpInputConnections[iConnection]->getRTCPConnection() == piRTCPConnection) 
      {
// We are supposed to ignore the media of the latter of two terminals
// whose SSRC collides
         MpRtpInputAudioConnection::stopReceiveRtp(*(getMsgQ()),
                                mpInputConnections[iConnection]->getName());
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();


}
#endif /* INCLUDE_RTCP ] */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

// Returns TRUE if the indicated codec is supported.
UtlBoolean MpCallFlowGraph::isCodecSupported(SdpCodec& rCodec)
{
   // $$$ For now always return TRUE
   return TRUE;
}

// Returns TRUE if the GIPS premium sound is enabled
UtlBoolean MpCallFlowGraph::isPremiumSoundEnabled(void)
{
   return mPremiumSoundEnabled;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
UtlBoolean MpCallFlowGraph::writeWAVHeader(int handle)
{
    UtlBoolean retCode = FALSE;
    char tmpbuf[80];
    short bitsPerSample = 16;

    short sampleSize = sizeof(MpAudioSample); 
    short compressionCode = 1; //PCM
    short numChannels = 1; 
    unsigned long samplesPerSecond = 8000;
    unsigned long averageSamplePerSec = samplesPerSecond*sampleSize;
    short blockAlign = sampleSize*numChannels; 
    unsigned long bytesWritten = 0;

    //write RIFF & length
    //8 bytes written
    strcpy(tmpbuf,"RIFF");
    unsigned long length = 0;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close
    
    //write WAVE & length
    //8 bytes written
    strcpy(tmpbuf,"WAVE");
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
//    bytesWritten += write(handle,&length, sizeof(length)); //filled in on close

    //write fmt & length
    //8 bytes written
    strcpy(tmpbuf,"fmt ");
    length = 16;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length,sizeof(length)); //filled in on close
    
    //now write each piece of the format
    //16 bytes written
    bytesWritten += write(handle, (char*)&compressionCode, sizeof(compressionCode));
    bytesWritten += write(handle, (char*)&numChannels, sizeof(numChannels));
    bytesWritten += write(handle, (char*)&samplesPerSecond, sizeof(samplesPerSecond));
    bytesWritten += write(handle, (char*)&averageSamplePerSec, sizeof(averageSamplePerSec));
    bytesWritten += write(handle, (char*)&blockAlign, sizeof(blockAlign));
    bytesWritten += write(handle, (char*)&bitsPerSample, sizeof(bitsPerSample));


    //write data and length
    strcpy(tmpbuf,"data");
    length = 0;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close
    
    //total length at this point should be 44 bytes
    if (bytesWritten == 44)
        retCode = TRUE;

    return retCode;

}


// Handles an incoming message for the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleMessage(OsMsg& rMsg)
{
   UtlBoolean retCode;

   retCode = FALSE;

   if (rMsg.getMsgType() == OsMsg::STREAMING_MSG)
   {
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
      case MpFlowGraphMsg::FLOWGRAPH_START_RECORD:
         retCode = handleStartRecord(*pMsg);
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
      case MpFlowGraphMsg::FLOWGRAPH_SET_PREMIUM_SOUND:
         retCode = handleSetPremiumSound(*pMsg);
         break;
      case MpFlowGraphMsg::FLOWGRAPH_SET_DTMF_NOTIFY:
         retCode = handleSetDtmfNotify(*pMsg);
         break;
      case MpFlowGraphMsg::ON_MPRRECORDER_ENABLED:
         retCode = handleOnMprRecorderEnabled(*pMsg);
         break;
      case MpFlowGraphMsg::ON_MPRRECORDER_DISABLED:
         retCode = handleOnMprRecorderDisabled(*pMsg);
         break;
      default:
         retCode = MpFlowGraphBase::handleMessage(*pMsg);
         break;
      }
   }

   return retCode;
}

// Handle the FLOWGRAPH_REMOVE_CONNECTION message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleRemoveConnection(MpFlowGraphMsg& rMsg)
{
   MpConnectionID connID = rMsg.getInt1();
   MpRtpInputAudioConnection* pInputConnection;
   MpRtpOutputAudioConnection* pOutputConnection;
   UtlBoolean    res;

   // Don't think this is needed as we make the ports available by
   // releasing the ports on the connected resources
   //mpBridge->disconnectPort(connID);
   mConnTableLock.acquire();
   pInputConnection = mpInputConnections[connID];
   pOutputConnection = mpOutputConnections[connID];
   mpInputConnections[connID] = NULL;
   mpOutputConnections[connID] = NULL;
   mConnTableLock.release();

   // now remove synchronous resources from flow graph
   if(pInputConnection)
   {
       res = handleRemoveResource(pInputConnection);
       assert(res);
       delete pInputConnection;
   }

   if(pOutputConnection)
   {
       res = handleRemoveResource(pOutputConnection);
       assert(res);
       delete pOutputConnection;
   }   
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartPlay(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   boolRes = mpFromFile->enable();          assert(boolRes);

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

UtlBoolean MpCallFlowGraph::handleStartRecord(MpFlowGraphMsg& rMsg)
{
   int i;
   int startPlayer = rMsg.getInt2();

   if (START_PLAY_FILE == startPlayer) handleStartPlay(rMsg);
   for (i=0; i<MAX_RECORDERS; i++) {
      if (NULL != mpRecorders[i]) {
         mpRecorders[i]->begin();
      }
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStopToneOrPlay()
{
   UtlBoolean boolRes;

#ifdef DOING_ECHO_CANCELATION /* [ */
   MpMediaTask* pMediaTask;

   pMediaTask = MpMediaTask::getMediaTask(0);
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

UtlBoolean MpCallFlowGraph::handleSetPremiumSound(MpFlowGraphMsg& rMsg)
{
   UtlBoolean save = mPremiumSoundEnabled;
   MpRtpInputAudioConnection::PremiumSoundOptions op =
       (MpRtpInputAudioConnection::PremiumSoundOptions) rMsg.getInt1();
   int i;

   mPremiumSoundEnabled = (op == MpRtpInputAudioConnection::EnablePremiumSound);
   if (save != mPremiumSoundEnabled) {
/*
      osPrintf("MpCallFlowGraph(%p): Premium sound %sABLED\n",
         this, mPremiumSoundEnabled ? "EN" : "DIS");
*/

      for (i=0; i<MAX_CONNECTIONS; i++) {
         if (NULL != mpInputConnections[i]) {
            mpInputConnections[i]->setPremiumSound(op);
         }
      }
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleSetDtmfNotify(MpFlowGraphMsg& rMsg)
{
   OsNotification* pNotify = (OsNotification*) rMsg.getPtr1();
   MpConnectionID  connId  = rMsg.getInt1();

   return mpInputConnections[connId]->handleSetDtmfNotify(pNotify);
}


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

UtlBoolean MpCallFlowGraph::handleOnMprRecorderEnabled(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes; 
   int status = rMsg.getInt1();
   MprRecorder* pRecorder = (MprRecorder*) rMsg.getPtr1() ;

   // if this call recorder, also enable required resources
   if (pRecorder && pRecorder == mpRecorders[RECORDER_CALL])
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

UtlBoolean MpCallFlowGraph::handleOnMprRecorderDisabled(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int status = rMsg.getInt1();
   MprRecorder* pRecorder = (MprRecorder*) rMsg.getPtr1() ;

   if (pRecorder && pRecorder == mpRecorders[RECORDER_CALL])
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


