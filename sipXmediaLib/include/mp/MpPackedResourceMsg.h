//  
// Copyright (C) 2009 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpPackedResourceMsg_h_
#define _MpPackedResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include "utl/UtlSerialized.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message used pass arbitrary data to a resource.
class MpPackedResourceMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpPackedResourceMsg(MpResourceMsgType msgType,
                       const UtlString& targetResourceName)
   : MpResourceMsg(msgType, targetResourceName)
   {
   };

     /// Copy constructor
   MpPackedResourceMsg(const MpPackedResourceMsg& resourceMsg)
   : MpResourceMsg(resourceMsg)
   , mData(resourceMsg.mData)
   {
   };

     /// @copydoc OsMsg::createCopy()
   OsMsg* createCopy() const 
   {
      return new MpPackedResourceMsg(*this); 
   }

     /// Destructor
   ~MpPackedResourceMsg() 
   {
   };

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MpPackedResourceMsg& operator=(const MpPackedResourceMsg& rhs)
   {
      if(&rhs == this)
      {
         return(*this);
      }

      MpResourceMsg::operator=(rhs);
      mData = rhs.mData;
      return *this;
   }

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get stored data.
   inline UtlSerialized &getData();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlSerialized    mData;     ///< Packed data to be passed to the resource,
};

/* ============================ INLINE METHODS ============================ */

UtlSerialized &MpPackedResourceMsg::getData()
{
   return mData;
}
#endif  // _MpPackedResourceMsg_h_
