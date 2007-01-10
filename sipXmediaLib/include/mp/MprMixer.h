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


#ifndef _MprMixer_h_
#define _MprMixer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "Mixer" media processing resource
*/
class MprMixer : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprMixer(const UtlString& rName, int numWeights,
            int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprMixer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Sets the weighting factors for the first "numWeights" inputs.
   UtlBoolean setWeights(int *newWeights, int numWeights);
     /**<
     *  For now, this method always returns TRUE.
     */

     /// Sets the weighting factor for the "weightIndex" input.
   UtlBoolean setWeight(int newWeight, int weightIndex);
     /**<
     *  For now, this method always returns TRUE.
     */

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

   typedef enum
   {
      SET_WEIGHT  = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_WEIGHTS
   } AddlMsgTypes;

   enum { MAX_MIXER_INPUTS = 10 };

   int mWeights[MAX_MIXER_INPUTS];
   int mNumWeights;
   int mScale;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle the @link MprMixer::SET_WEIGHT SET_WEIGHT @endlink message.
   UtlBoolean handleSetWeight(int newWeight, int weightIndex);

     /// Handle the @link MprMixer::SET_WEIGHTS SET_WEIGHTS @endlink message.
   UtlBoolean handleSetWeights(int *newWeights, int numWeights);

     /// Copy constructor (not implemented for this class)
   MprMixer(const MprMixer& rMprMixer);

     /// Assignment operator (not implemented for this class)
   MprMixer& operator=(const MprMixer& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprMixer_h_
