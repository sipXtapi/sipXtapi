//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprnProgressMsg_h_
#define _MprnProgressMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "MpResNotificationMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

  /// Message notification object used to communicate millisecond-based progress 
  /// updates from resources outward towards the flowgraph, and up through to 
  /// users above mediaLib and beyond.
class MprnProgressMsg : public MpResNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprnProgressMsg(MpResNotificationMsg::RNMsgType msgType,
                   const UtlString& namedResOriginator,
                   unsigned posMS, unsigned totalMS);

   /// Copy constructor
   MprnProgressMsg(const MprnProgressMsg& rMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual
      ~MprnProgressMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprnProgressMsg& operator=(const MprnProgressMsg& rhs);

     /// Set the position in milliseconds this message reports.
   void setPositionMS(unsigned posMS);

     /// Set the total size of the buffer in milliseconds this message reports.
   void setTotalMS(unsigned totalMS);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the current reported position of this progress update in MS.
   unsigned getPositionMS(void) const;

     /// Get the total size of the buffer in milliseconds this message reports.
   unsigned getTotalMS(void) const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   unsigned mCurPositionMS; ///< current position reported, in milliseconds.
   unsigned mTotalMS; ///< Total size in milliseconds.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprnProgressMsg_h_
