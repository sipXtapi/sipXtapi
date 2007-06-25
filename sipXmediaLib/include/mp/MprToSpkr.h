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


#ifndef _MprToSpkr_h_
#define _MprToSpkr_h_

#define DETECT_SPKR_OVERFLOW
#undef DETECT_SPKR_OVERFLOW
// #define DETECT_SPKR_OVERFLOW  //MG 10-3-01, trying to understand where the signal clips

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsNotification.h"
#include "os/OsMsgQ.h"
//#include "mp/MpMisc.h"
#include "mp/MpAudioResource.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*TOSPEAKERHOOK)(const int nLength, short* samples) ;
// FORWARD DECLARATIONS

/**
*  @brief The "To Speaker" media processing resource.
*
*/
class MprToSpkr : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum{VOLUME_CONTROL_TABLE_SIZE=64};
   enum{MIN_SPKR_DtoA=-32767};
   enum{MAX_SPKR_DtoA= 32767};

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprToSpkr(const UtlString& rName, int samplesPerFrame, int samplesPerSec,
             OsMsgQ *pSpkQ, OsMsgQ *pEchoQ);

     /// Destructor
   virtual
   ~MprToSpkr();

   // I had to increase this on Win/NT because of the bursty nature of
   // the completion callbacks:  the waveOut operations send completion
   // acknowledgements in bursts covering 60 to 100 msecs at once.  At
   // 10 msec per buffer, this resulted in frequent starvation.
   enum { MAX_SPKR_BUFFERS = 12 };
   enum { MIN_SPKR_BUFFERS = 1 };
   enum { SKIP_SPKR_BUFFERS = 1 };

#ifdef REIMPLEMENT_CLARISIS_EQ /* [ */
   enum {EqFilterLen_ix = 24};

   static       int  smClarisisHandsetSpeakerEq[EqFilterLen_ix];
   int*              mpEqSave_ix;
   int               mLastSpkr_ix;
   int*              mpCurEq_ix;

   void SpeakerEqualization_ix(MpAudioSample* samples, int iLength);
#endif /* REIMPLEMENT_CLARISIS_EQ ] */

   typedef enum {
      ATTEN_LOUDEST = 0,    ///< 0 dB, no attenuation
      ATTEN_QUIETEST = -6   ///< Please do not make this lower than -48
   } AttenValues;

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

//@}

#ifdef DETECT_SPKR_OVERFLOW /* [ */
    static int spkrStats();
#endif /* DETECT_SPKR_OVERFLOW ] */

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      PLAY_FILE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START
   } AddlMsgTypes;

   enum{EqFilterLen = 24};


   OsMsgQ*         mpSpkQ;        ///< Audio data will be sent to this queue.
   OsMsgQ*         mpEchoQ;       ///< Audio data will be sent to this queue too.
                                  ///<  This queue should be connected to Echo
                                  ///<  Cancelation resource.

   static int      slInitVol;
   static int      slVolStep;
   uint32_t        mulNoiseLevel; ///< Used in comfort noise generation.

   enum{MAX_SUPPRESSION = 7};
   int             mlpVolTable[VOLUME_CONTROL_TABLE_SIZE];
                                     ///< volume levels + a mute level

#ifdef DETECT_SPKR_OVERFLOW // [

   static int smStatsReports;

   int        mOverflowsIn;
   int        mUnderflowsIn;
   int        mOverflowsOut;
   int        mUnderflowsOut;
   int        mTotalSamples;
   int        mMaxIn;
   int        mMinIn;
   int        mMaxOut;
   int        mMinOut;
   int        mReport;

   void stats(void);

#endif // DETECT_SPKR_OVERFLOW ]

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

   void initVolTable(void);

     /// Copy constructor (not implemented for this class)
   MprToSpkr(const MprToSpkr& rMprToSpkr);

     /// Assignment operator (not implemented for this class)
   MprToSpkr& operator=(const MprToSpkr& rhs);

public:
    static TOSPEAKERHOOK s_fnToSpeakerHook ;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToSpkr_h_
