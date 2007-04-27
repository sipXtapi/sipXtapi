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
   MpToneResourceMsg(const UtlString& msgDestName, int toneId)
      : MpResourceMsg(MPRM_START_TONE, msgDestName)
      , mToneId(toneId) 
   {};

   /// Copy constructor
   MpToneResourceMsg(const MpToneResourceMsg& rMpResourceMsg)
      : MpResourceMsg(rMpResourceMsg)
      , mToneId(rMpResourceMsg.mToneId)
   {};

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MpToneResourceMsg(*this); 
   }

   /// Destructor
   ~MpToneResourceMsg() { /* No work required */ };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MpToneResourceMsg& operator=(const MpToneResourceMsg& rhs)
   {
      if (this == &rhs) 
         return *this;  // handle the assignment to self case

      MpResourceMsg::operator=(rhs);  // assign fields for parent class
      mToneId = rhs.mToneId;
      return *this;
   }

     /// @brief Set the tone that is associated with this resource.
   inline void setToneId(int toneId) { mToneId = toneId; }
     /**<
     *  Set the tone that is associated with this resource.
     *  @param toneId the new tone that is to be set in this resource.
     */

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the tone that is associated with this resource.
   inline int getToneId(void) const { return mToneId; };
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
   int mToneId; ///< The tone to play/stop/etc.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpToneResourceMsg_h_
