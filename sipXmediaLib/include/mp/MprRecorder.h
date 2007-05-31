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


#ifndef _MprRecorder_h_
#define _MprRecorder_h_

// SYSTEM INCLUDES
#ifdef _VXWORKS /* [ */
#   include <ioLib.h>
#endif /* _VXWORKS ] */

#if defined(_WIN32) && !defined(WINCE) /* [ */
#   include <io.h>
#endif /* _WIN32 ] */

// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"
#include "os/OsProtectEvent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

struct MprRecorderStats
{
   double mTotalBytesWritten;
   double mTotalSamplesWritten;
   double mDuration;
   int mFinalStatus;
   int mDtmfTerm;
};

// TYPEDEFS

// FORWARD DECLARATIONS

/// The "Recorder" media processing resource
class MprRecorder : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum {
      UNINITIALIZED_FORMAT = -1,
      RAW_PCM_16 = 0,
      WAV_PCM_16
   }  RecordFileFormat;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprRecorder(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprRecorder();

   typedef enum
   {
      RECORD_FINISHED,
      RECORD_STOPPED,
      RECORDING,
      WRITE_ERROR,
      RECORD_IDLE,
      INVALID_SETUP
   }Completion;

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Set parameters for next start; MUST BE disabled when this is called
   virtual OsStatus setup(int file, RecordFileFormat recFormat, int time = 0,
                          int silenceLength = 5, OsEvent* event = NULL);
     /**<
     *  @param file - destination for record
     *  @param recFormat - output format type (e.g. WAV_PCM_16)
     *  @param time - max number of milliseconds to record, or 0 for no limit
     *  @param silenceLength - (in seconds)
     *  @param event - an optional OsEvent to signal on completion.
     */

   virtual OsStatus begin(void);

   virtual OsStatus stop(void);

   virtual UtlBoolean disable(Completion code);

   virtual UtlBoolean enable(void);

   virtual UtlBoolean termDtmf(int currentToneKey);
  
   UtlBoolean closeRecorder();

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   void getRecorderStats(double& nBytes, double& nSamples, Completion& status);
   void getRecorderStats(struct MprRecorderStats* p);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
  UtlBoolean updateWaveHeaderLengths(int handle);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef enum
   {
      BEGIN = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP,
      SETUP
   } AddlMsgTypes;

   int mTermKey;
   int mFileDescriptor;
   RecordFileFormat mRecFormat;
   double mTotalBytesWritten;
   double mTotalSamplesWritten;
   int mConsecutiveInactive;
   int mSilenceLength;
   long mPrevEnergy;
   OsProtectedEvent* mpEvent;
   int mFramesToRecord;
   Completion mStatus;
   OsMutex mMutex;


   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprRecorder(const MprRecorder& rMprRecorder);

     /// Assignment operator (not implemented for this class)
   MprRecorder& operator=(const MprRecorder& rhs);

   UtlBoolean handleSetup(int file, int time, int silenceLength, OsProtectedEvent* event);

   UtlBoolean handleBegin(void);

   UtlBoolean handleStop(void);

   void progressReport(Completion code);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRecorder_h_
