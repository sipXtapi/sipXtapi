// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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

#ifndef _MiDtmfNotf_h_
#define _MiDtmfNotf_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mi/MiNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

  /// Message notification class used to communicate DTMF signaling.
  ///
  /// If one creates a KEY_DOWN notification, duration other than 
  /// DURATION_NOT_APPLICABLE should not be supplied, as it isn't 
  /// useful until a KEY_UP event happens.
class MiDtmfNotf : public MiNotification
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int32_t DURATION_NOT_APPLICABLE;

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
   MiDtmfNotf(const UtlString& sourceId, int connId, KeyCode key, 
              KeyPressState pressState, int32_t duration = DURATION_NOT_APPLICABLE);

   /// Copy constructor
   MiDtmfNotf(const MiDtmfNotf& rNotf);

   /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy(void) const;

   /// Destructor
   virtual ~MiDtmfNotf();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MiDtmfNotf& operator=(const MiDtmfNotf& rhs);

     /// Set the specific DTMF value that this notification represents.
   void setKeyCode(KeyCode key);

     /// Set the key press state for this DTMF notification -- down or up.
   void setKeyPressState(KeyPressState pressState);

     /// Set the duration of this DTMF notification.
   void setDuration(int32_t duration);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

     /// Get the specific DTMF value that this notification represents.
   KeyCode getKeyCode() const;

     /// Get the key press state for this DTMF notification -- down or up.
   KeyPressState getKeyPressState() const;

     /// Get the duration of this DTMF notification.
   int32_t getDuration() const;

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   UtlBoolean isPressed() const;

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   KeyCode mKey;   ///< The DTMF key value.
   KeyPressState mPressState;  ///< Whether the key is up or down.
   int32_t mDuration;  ///< Duration of the DTMF event.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiDtmfNotf_h_
