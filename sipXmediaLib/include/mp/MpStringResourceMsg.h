//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpStringResourceMsg_h_
#define _MpStringResourceMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message used to pass a string to resource.
class MpStringResourceMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpStringResourceMsg(MpResourceMsgType type,
                       const UtlString& targetResourceName,
                       const UtlString& data)
   : MpResourceMsg(type, targetResourceName)
   , mData(data)
   {
   };

     /// Copy constructor
   MpStringResourceMsg(const MpStringResourceMsg &msg)
   : MpResourceMsg(msg)
   , mData(msg.mData)
   {
   };

     /// @copydoc MpResourceMsg::createCopy()
   OsMsg* createCopy() const 
   {
      return new MpStringResourceMsg(*this); 
   }

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   /// Assignment operator
   MpStringResourceMsg& operator=(const MpStringResourceMsg& rhs)
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

     /// Return contained string.
   const UtlString &getData() const {return mData;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlString mData;      ///< String to be passed to resource.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpStringResourceMsg_h_
