//  
// Copyright (C) 2006-2011 SIPfoundry Inc.  All rights reserved.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprRtpInputConnectionConstructor_h_
#define _MprRtpInputConnectionConstructor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResourceConstructor.h>
#include <mp/MpRtpInputConnection.h>
#include <mp/MprRtpDispatcher.h>
#include <mp/MprToSpkr.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief MprRtpInputConnectionConstructor is used to construct
*         a MprRtpInputConnection resource
*/
class MprRtpInputConnectionConstructor : public MpAudioResourceConstructor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   MprRtpInputConnectionConstructor(UtlBoolean isMcast = FALSE,
                                         int numRtpStreams = 1)
   : MpAudioResourceConstructor(isMcast ? DEFAULT_MCAST_RTP_INPUT_RESOURCE_TYPE
                                        : DEFAULT_RTP_INPUT_RESOURCE_TYPE,
                                0, 0, //minInputs, maxInputs,
                                0, numRtpStreams) //minOutputs, maxOutputs
   , mNumRtpStreams(numRtpStreams)
   , mIsMcast(isMcast)
   {
   }

     /// Destructor
   virtual ~MprRtpInputConnectionConstructor() {}

/* ============================ MANIPULATORS ============================== */

   /// Create a new resource
   virtual OsStatus newResource(const UtlString& resourceName,
                                int maxResourcesToCreate,
                                int& numResourcesCreated,
                                MpResource* resourceArray[])
   {
      assert(maxResourcesToCreate >= 1);
      numResourcesCreated = 1;
      MprRtpDispatcher::RtpStreamAffinity streamAffinity =
         mIsMcast ? MprRtpDispatcher::MOST_RECENT_SSRC
                  : MprRtpDispatcher::ADDRESS_AND_PORT;
      resourceArray[0] = new MpRtpInputConnection(resourceName,
                                                  -1,
                                                  NULL,
                                                  mNumRtpStreams,
                                                  streamAffinity);
      resourceArray[0]->enable();
      return(OS_SUCCESS);
   }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int mNumRtpStreams;
   UtlBoolean mIsMcast;

     /// Disabled copy constructor
   MprRtpInputConnectionConstructor(const MprRtpInputConnectionConstructor& rMprRtpInputConnectionConstructor);


     /// Disabled assignment operator
   MprRtpInputConnectionConstructor& operator=(const MprRtpInputConnectionConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpInputConnectionConstructor_h_
