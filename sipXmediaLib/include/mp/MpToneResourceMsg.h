//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpToneResourceMsg_h_
#define _MpToneResourceMsg_h_

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

/// Message object used to communicate with the media processing task
class MpToneResourceMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   inline MpToneResourceMsg(const UtlString& msgDestName, int toneCode);

   /// Copy constructor
   inline MpToneResourceMsg(const MpToneResourceMsg& rMpResourceMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   inline OsMsg* createCopy(void) const;

   /// Destructor
   inline ~MpToneResourceMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   inline MpToneResourceMsg& operator=(const MpToneResourceMsg& rhs);

     /// @brief Set the tone that is associated with this resource.
   inline void setToneCode(int toneCode);
     /**<
     *  Set the tone that is associated with this resource.
     *  @param toneId the new tone that is to be set in this resource.
     */

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the tone that is associated with this resource.
   inline int getToneCode(void) const;
     /**<
     *  Returns the tone that is associated with this resource.
     */

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mToneCode; ///< The tone to play/stop/etc.
};

/* ============================ INLINE METHODS ============================ */

MpToneResourceMsg::MpToneResourceMsg(const UtlString& msgDestName, 
                                     int toneCode)
: MpResourceMsg(MPRM_START_TONE, msgDestName)
, mToneCode(toneCode)
{
}

MpToneResourceMsg::MpToneResourceMsg(const MpToneResourceMsg& rMpResourceMsg)
: MpResourceMsg(rMpResourceMsg)
, mToneCode(rMpResourceMsg.mToneCode)
{};

OsMsg* MpToneResourceMsg::createCopy(void) const 
{
   return new MpToneResourceMsg(*this); 
}

MpToneResourceMsg::~MpToneResourceMsg() 
{ 
   /* No work required */ 
}

MpToneResourceMsg& 
MpToneResourceMsg::operator=(const MpToneResourceMsg& rhs)
{
   if (this == &rhs) 
      return *this;  // handle the assignment to self case

   MpResourceMsg::operator=(rhs);  // assign fields for parent class
   mToneCode = rhs.mToneCode;
   return *this;
}

void MpToneResourceMsg::setToneCode(int toneCode) 
{ 
   mToneCode = toneCode; 
}

int MpToneResourceMsg::getToneCode(void) const 
{ 
   return mToneCode; 
}

#endif  // _MpToneResourceMsg_h_
