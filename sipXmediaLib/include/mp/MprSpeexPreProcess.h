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

/// The "Speex Audio Preprocessor" media processing resource
/**
*  This resource is a wrapper over Speex's audio preprocessor. It is used
*  to do Automatic Gain Control, denoising and echo residue removal.
*  
*  MprSpeexPreprocess expects audio data on the first input and and produces
*  processed audio on its first output. Residual echo removal is activated
*  if MprSpeexPreprocess input is connected directly to MprSpeexEchoCancel
*  output.
*/
class MprSpeexPreprocess : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum GlobalEnableState {
      GLOBAL_ENABLE,   ///< All MprSpeexEchoCancel resources are forced to enable
      GLOBAL_DISABLE,  ///< All MprSpeexEchoCancel resources are forced to disable
      LOCAL_MODE       ///< Each resource respect its own enable/disable state
   };

/* ============================ CREATORS ================================== */

     /// Constructor
   MprSpeexPreprocess(const UtlString& rName, UtlBoolean isAgcEnabled=FALSE,
                      UtlBoolean isNoiseReductionEnabled=FALSE);

     /// Destructor
   virtual
   ~MprSpeexPreprocess();


/* ============================ MANIPULATORS ============================== */

     /// Enable or disable Automatic Gain Control
   UtlBoolean setAGC(UtlBoolean enable);

     /// Enable or disable noise reduction
   UtlBoolean setNoiseReduction(UtlBoolean enable);

     /// Set global AGC enable/disable state.
   static inline void setGlobalAgcEnableState(GlobalEnableState state);
     /**<
     *  @see smGlobalAgcEnableState for more details.
     */

     /// Set global Noise Reduction enable/disable state.
   static inline void setGlobalNoiseReductionEnableState(GlobalEnableState state);
     /**<
     *  @see smGlobalNoiseReductionEnableState for more details.
     */

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
   UtlBoolean mIsAgcEnabled;                ///< Should AGC be enabled?
   UtlBoolean mIsNoiseReductionEnabled;     ///< Should Noise Reduction be enabled?

   static volatile GlobalEnableState smGlobalAgcEnableState;
     ///< Global enable/disable switch for all Speex AGC resources. We need
     ///< this switch because sipXmediaAdapterLib exports only a static method
     ///< to turn AGC on and off.
   UtlBoolean mIsAgcEnabledReal; ///< Is AGC really enabled?
   static volatile GlobalEnableState smGlobalNoiseReductionEnableState;
     ///< Global enable/disable switch for all Speex Noise Reduction resources.
     ///< We need this switch because sipXmediaAdapterLib exports only a static
     ///< method to turn Noise Reduction on and off.
   UtlBoolean mIsNoiseReductionEnabledReal; ///< Is Noise Reduction really enabled?

   inline UtlBoolean getRealAgcState() const;
   inline UtlBoolean getRealNoiseReductionState() const;

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

     /// @brief Associates this resource with the indicated flow graph.
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  We use this overloaded method for initialization of some of our member
     *  variables, which depend on flowgraph's properties (like frame size).
     *
     *  @retval OS_SUCCESS - for now, this method always returns success
     */

     /// @copydoc MpResource::connectInput()
   UtlBoolean connectInput(MpResource& rFrom, int fromPortIdx, int toPortIdx);

     /// @copydoc MpResource::disconnectInput()
   UtlBoolean disconnectInput(int outPortIdx);

     /// Copy constructor (not implemented for this class)
   MprSpeexPreprocess(const MprSpeexPreprocess& rMprSpeexPreprocess);

     /// Assignment operator (not implemented for this class)
   MprSpeexPreprocess& operator=(const MprSpeexPreprocess& rhs);

};

void MprSpeexPreprocess::setGlobalAgcEnableState(GlobalEnableState state)
{
   smGlobalAgcEnableState = state;
}

void MprSpeexPreprocess::setGlobalNoiseReductionEnableState(GlobalEnableState state)
{
   smGlobalNoiseReductionEnableState = state;
}

UtlBoolean MprSpeexPreprocess::getRealAgcState() const
{
   return (smGlobalAgcEnableState==LOCAL_MODE) ? mIsAgcEnabled
          : (smGlobalAgcEnableState==GLOBAL_ENABLE);
}

UtlBoolean MprSpeexPreprocess::getRealNoiseReductionState() const
{
   return (smGlobalNoiseReductionEnableState==LOCAL_MODE) ? mIsNoiseReductionEnabled
          : (smGlobalAgcEnableState==GLOBAL_ENABLE);
}

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeexPreprocess_h_
