//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpIntResourceMsg_h_
#define _MpIntResourceMsg_h_

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

/// Message used to pass an integer value to resource.
class MpIntResourceMsg : public MpResourceMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpIntResourceMsg(MpResourceMsgType type,
                    const UtlString& targetResourceName,
                    int data)
   : MpResourceMsg(type, targetResourceName)
   , mData(data)
   {
   };

     /// Copy constructor
   MpIntResourceMsg(const MpIntResourceMsg &msg)
   : MpResourceMsg(msg)
   , mData(msg.mData)
   {
   };

     /// @copydoc MpResourceMsg::createCopy()
   OsMsg* createCopy() const
   {
      return new MpIntResourceMsg(*this); 
   }

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   /// Assignment operator
   MpIntResourceMsg& operator=(const MpIntResourceMsg& rhs)
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

     /// Return contained integer.
   int getData() const {return mData;}

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mData;      ///< Integer to be passed to resource.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpIntResourceMsg_h_
