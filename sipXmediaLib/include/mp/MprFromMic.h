// 
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
////////////////////////////////////////////////////////////////////////////////

#ifndef _MprFromMic_h_
#define _MprFromMic_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpResource.h"
#include "mp/MpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class DspResampling;

//:The "From Microphone" media processing resource
class MprFromMic : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

#ifdef _VXWORKS
   enum { MAX_MIC_BUFFERS = 2 }; /* NOT LESS THAN 1!! */
#endif
#if defined(_WIN32) || defined(__pingtel_on_posix__)
   enum { MAX_MIC_BUFFERS = 10 };
#endif

/* ============================ CREATORS ================================== */

   MprFromMic(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprFromMic();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

#ifdef _VXWORKS /* [ */
static int enableMicEq(int disable);
static int disableMicEq(int enable);
#endif /* _VXWORKS ] */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum{EqFilterLen = 24}; // Brant, 11 May 2001; was 13, allow for experiments

#ifdef _VXWORKS /* [ */
   static int      sbDoEqualization;
#endif /* _VXWORKS ] */

   short        shpFilterBuf[80 + 10];
   DspResampling* mpDspResamp;

#ifdef _VXWORKS /* [ */
   static       int  smHandsetEq[EqFilterLen]; //Rev B
   static       int  smHandsetNewEq[EqFilterLen]; //Rev C
   static       int  smHeadsetEq[EqFilterLen];
   static       int  smBaseEq[EqFilterLen];
   int*              mpEqSave;
   int               mLastMic;
   int*              mpCurEq;
   int               mEqFilterZeroState0;
   int               mEqFilterZeroState1;
   int               mEqFilterZeroState2;
   int               mEqFilterPoleState0;
   int               mEqFilterPoleState1;
   int               mEqFilterPoleState2;

   static MpCodecMicChoice smForceMic;
   static MpCodecMicChoice forceMic(MpCodecMicChoice what);
#endif /* _VXWORKS ] */

   int               mNumEmpties;
   int               mNumFrames;

   void  Init_highpass_filter800();
   void  highpass_filter800(short *, short *, short);
   short speech_detected(short*, int);

#ifdef _VXWORKS /* [ */
   void equalization(Sample* samples, int iLength);

         int* chooseEqFilter(void);
#endif /* _VXWORKS ] */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprFromMic(const MprFromMic& rMprFromMic);
     //:Copy constructor (not implemented for this class)

   MprFromMic& operator=(const MprFromMic& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromMic_h_
