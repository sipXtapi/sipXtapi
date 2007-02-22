// 
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004-2007 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _OsTimerMsg_h_
#define _OsTimerMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsRpcMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsTimer;

//:Messages used to request timer services

class OsTimerMsg : public OsRpcMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgSubType
   {
      OS_TIMER_UPDATE,          ///< update the status of the timer
      OS_TIMER_UPDATE_SYNC,     /**< update the status of the timer and signal
                                 *   an event object
                                 */
      OS_TIMER_UPDATE_DELETE,   /**< update the status of the timer and
                                 *   delete it
                                 */
      OS_TIMER_SHUTDOWN         ///< shut down the timer task */
   };

/* ============================ CREATORS ================================== */

   OsTimerMsg(const unsigned char subType,
              OsTimer* pTimer,
              OsEvent* pEvent);
     //:Constructor

   OsTimerMsg(const OsTimerMsg& rOsTimerMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~OsTimerMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsTimerMsg& operator=(const OsTimerMsg& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   /// Return the (pointer to the OsTimer object) in this message.
   inline OsTimer* getTimerP(void) const
   {
      return mpTimer;
   }

   /// Return the (pointer to the OsEvent object) in this message.
   inline OsEvent* getEventP(void) const
   {
      return OsRpcMsg::getEvent();
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsTimer* mpTimer;

   void init(void);
     //:Initialization common to all constructors

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsTimerMsg_h_
