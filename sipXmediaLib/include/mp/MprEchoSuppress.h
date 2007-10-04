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

#ifndef _MprEchoSuppress_h_
#define _MprEchoSuppress_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class MprToSpkr;
class FilterBank;
class HandsetFilterBank;

/**
*  @brief The "Echo suppress" media processing resource
*/
class MprEchoSuppress : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Manipulators
//@{

     /// Constructor
   MprEchoSuppress(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprEchoSuppress();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   void setSpkrPal(MprToSpkr* pal);

   int startSpeech();
   int endSpeech();

//@}

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
   short                mState;
   MprToSpkr*           mpSpkrPal;
   MpAudioBufPtr        mpPrev;
   int                  mTicksPerFrame;
   int                  mLastSpkrAtten;
   int                  mSpeechFake;
   short                mshDelay;
   FilterBank*          mpFilterBank;
   HandsetFilterBank*   mpHandsetFilterBank;

   MpBufPtr LoudspeakerFade(MpBufPtr in, short& shSpkState, int iFreezeFlag);

   void frame_match(const MpAudioBufPtr &in);

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   void control_logic(unsigned long       ulSigIn,
                      unsigned long       ulSigOut,
                              short&      shSpkState,
                                int       iFreezeFlag);

     /// Copy constructor (not implemented for this class)
   MprEchoSuppress(const MprEchoSuppress& rMprEchoSuppress);

     /// Assignment operator (not implemented for this class)
   MprEchoSuppress& operator=(const MprEchoSuppress& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprEchoSuppress_h_
