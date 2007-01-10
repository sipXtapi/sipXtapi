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


#ifndef _MprBridge_h_
#define _MprBridge_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpAudioConnection.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The conference bridge resource.
*
*  Local input and output should be connected to the first input and output
*  ports. Remote connections will be binded to the second input/output ports,
*  third input/output ports, and so on.
*
*  <H3>Enabled behaviour</H3>
*  Mix together local and remote inputs onto outputs, with the requirement that
*  no output receive its own input.
*
*  <H3>Disabled behaviour</H3>
*  Mix all remote inputs onto local speaker, and copy our local microphone to
*  all remote outputs.
*/
class MprBridge : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MprBridge(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprBridge();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Attach MpAudioConnection to an available port.
   int connectPort(MpConnectionID connID);

     /// Disconnect MpAudioConnection from its port.
   OsStatus disconnectPort(MpConnectionID connID);

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

    UtlBoolean doMix(MpAudioBufPtr inBufs[], int inBufsSize,
                     MpAudioBufPtr &out, int samplesPerFrame) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum { MAX_BRIDGE_PORTS = 10 };

   MpConnectionID mpConnectionIDs[MAX_BRIDGE_PORTS];
                              ///< IDs of remote connections, binded to this
                              ///< bridge.
   OsBSem         mPortLock;  ///< Mutex for ports access synchronization

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Find and return the index to an unused port pair
   int findFreePort(void);

     /// Check whether this port is connected to both input and output
   UtlBoolean isPortActive(int portIdx) const;

     /// Copy constructor (not implemented for this class)
   MprBridge(const MprBridge& rMprBridge);

     /// Assignment operator (not implemented for this class)
   MprBridge& operator=(const MprBridge& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridge_h_
