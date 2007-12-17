//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _MprSpeexPreprocess_h_
#define _MprSpeexPreprocess_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpFlowGraphMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
struct SpeexPreprocessState_;
typedef SpeexPreprocessState_ SpeexPreprocessState;

/// The "Speex Audio Prepocessor" media processing resource
/**
*  This resource is a wrapper over Speex's audio preprocessor. It is used
*  to do Automatic Gain Control, denoising and echo residue removal.
*  
*  MprSpeexPreprocess expects audio data on the first input and echo residue
*  power spectrum on the second input and produces processed audio on its
*  first output. Echo residue power spectrum could be get from second output
*  of MprSpeexEchoCancel resource.
*/
class MprSpeexPreprocess : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   MprSpeexPreprocess(const UtlString& rName,
                      int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprSpeexPreprocess();


/* ============================ MANIPULATORS ============================== */

     /// Enable or disable Automatic Gain Control
   UtlBoolean setAGC(UtlBoolean enable);

     /// Enable or disable noise reduction
   UtlBoolean setNoiseReduction(UtlBoolean enable);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      SET_AGC  = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_NOISE_REDUCTION
   } AddlMsgTypes;

   SpeexPreprocessState *mpPreprocessState; ///< Structure containing internal
                                            ///<  state of Speex preprocessor.

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle the @link MprSpeexPreprocess::SET_AGC SET_AGC @endlink message.
   UtlBoolean handleSetAGC(UtlBoolean enable);

     /// Handle the @link MprSpeexPreprocess::SET_NOISE_REDUCTION SET_NOISE_REDUCTION @endlink message.
   UtlBoolean handleSetNoiseReduction(UtlBoolean enable);

     /// Copy constructor (not implemented for this class)
   MprSpeexPreprocess(const MprSpeexPreprocess& rMprSpeexPreprocess);

     /// Assignment operator (not implemented for this class)
   MprSpeexPreprocess& operator=(const MprSpeexPreprocess& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeexPreprocess_h_