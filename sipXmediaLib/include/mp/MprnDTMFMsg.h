//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MprnDTMFMsg_h_
#define _MprnDTMFMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mp/MpTypes.h"
#include "MpResNotificationMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

  /// Message notification object used to communicate DTMF signaling from 
  /// resources outward towards the flowgraph, and up through to users above
  /// mediaLib and beyond.
class MprnDTMFMsg : public MpResNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum KeyPressState
   {
      KEY_UP,
      KEY_DOWN
   };

   enum KeyCode
   {
      DTMF_0 = 0,
      DTMF_1,
      DTMF_2,
      DTMF_3,
      DTMF_4,
      DTMF_5,
      DTMF_6,
      DTMF_7,
      DTMF_8,
      DTMF_9,
      DTMF_STAR = 10,
      DTMF_POUND,
      DTMF_A,
      DTMF_B,
      DTMF_C,
      DTMF_D
   };

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprnDTMFMsg(const UtlString& namedResOriginator, MpConnectionID connId, 
               KeyCode key, KeyPressState pressState, uint16_t duration);

   /// Copy constructor
   MprnDTMFMsg(const MprnDTMFMsg& rMsg);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual ~MprnDTMFMsg();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprnDTMFMsg& operator=(const MprnDTMFMsg& rhs);

     /// Set the specific DTMF value that this event represents.
   void setKeyCode(KeyCode key);

     /// Set the key press state for this DTMF message -- down or up.
   void setKeyPressState(KeyPressState pressState);

     /// Set the duration of this DTMF event.
   void setDuration(uint16_t duration);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the specific DTMF value that this event represents.
   KeyCode getKeyCode() const;

     /// Get the key press state for this DTMF message -- down or up.
   KeyPressState getKeyPressState() const;

     /// Get the duration of this DTMF event.
   uint16_t getDuration() const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

     /// Return TRUE if this notification indicates key is down/pressed.
   UtlBoolean isPressed() const;

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   KeyCode mKey;   ///< The DTMF key value.
   KeyPressState mPressState;  ///< Whether the key is up or down.
   uint16_t mDuration;  ///< Duration of the DTMF event.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprnDTMFMsg_h_
