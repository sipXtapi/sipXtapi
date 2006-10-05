//
//
// Copyright 2005 Pingtel Corp.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _LinePresenceMonitorMsg_h_
#define _LinePresenceMonitorMsg_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class LinePresenceMonitor;

//
// LinePresenceMonitorMsg
//
class LinePresenceMonitorMsg : public OsMsg {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum eLinePresenceMonitorMsgSubTypes {
      SUBSCRIBE_DIALOG,
      UNSUBSCRIBE_DIALOG,
      SUBSCRIBE_PRESENCE,
      UNSUBSCRIBE_PRESENCE
   };

/* ============================ CREATORS ================================== */

   // Constructor for UPDATE_STATE messages
   LinePresenceMonitorMsg(eLinePresenceMonitorMsgSubTypes type, LinePresenceBase* line);

   // Copy constructor
   LinePresenceMonitorMsg(const LinePresenceMonitorMsg& rLinePresenceMonitorMsg);

   // Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const;

   // Destructor
   virtual ~LinePresenceMonitorMsg();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   // Get the associated line
   LinePresenceBase* getLine(void) const { return mLine; }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   LinePresenceBase* mLine;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _LinePresenceMonitorMsg_h_
