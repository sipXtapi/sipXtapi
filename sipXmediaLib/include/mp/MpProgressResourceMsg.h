//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpProgressResourceMsg_h_
#define _MpProgressResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpResourceMsg.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate an update period with the media processing task
class MpProgressResourceMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

     /// Constructor
   MpProgressResourceMsg(MpResourceMsg::MpResourceMsgType msgType,
                       const UtlString& msgDestName, 
                       int32_t updatePeriodMS);
     /**<
     *  Main constructor for a Progress resource message.
     *  Initializes the progress resource message with a message type, 
     *  destination resource name, and a time-based update period (in MS).
     *  
     *  @NOTE Despite being a signed 32bit value, the \p updatePeriodMS 
     *        parameter should only be given positive values, as we want to have
     *        the upper bound be 2^31 to prevent issues with clock rollover.
     *  
     *  @param[in] msgType - the type of resource message to send
     *  @param[in] msgDestName - the name of a resource to send this message to.
     *  @param[in] updatePeriodMS - the update period to set - this should be a 
     *             positive value.
     */

     /// Copy constructor
   MpProgressResourceMsg(const MpProgressResourceMsg& rMsg);

     /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

     /// Destructor
   ~MpProgressResourceMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

     /// Assignment operator
   MpProgressResourceMsg& operator=(const MpProgressResourceMsg& rhs);

     /// Set the update period (in MS) that is associated with this resource.
   OsStatus setUpdatePeriodMS(int32_t updatePeriodMS);
     /**<
     *  Set the update period (in MS) that is associated with this resource.
     *  @NOTE Despite being a signed 32bit value, the \p updatePeriodMS
     *        parameter should only be given positive values, as we want to have 
     *        the upper bound be 2^31 to prevent issues with clock rollover.
     *  
     *  @param[in] updatePeriodMS - the update period to set - this should be a 
     *             positive value.
     */

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the update period (in MS) that is associated with this resource.
   int32_t getUpdatePeriodMS(void) const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int32_t mPeriodMS; ///< The (Positive) update period this message holds.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpProgressResourceMsg_h_
