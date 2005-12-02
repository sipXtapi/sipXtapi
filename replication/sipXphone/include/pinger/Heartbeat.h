// $Id: //depot/OPENDEV/sipXphone/include/pinger/Heartbeat.h#3 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HeartbeatTask_h_
#define _HeartbeatTask_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsBSem.h"
#include "os/OsConfigDb.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class HeartbeatTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   HeartbeatTask(OsConfigDb* pConfig);
     //:Default constructor


   virtual
   ~HeartbeatTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   static HeartbeatTask* getHeartbeatTask(OsConfigDb* pConfig);

	virtual int run(void* pArg);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // Singleton globals
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getTaoServerTask
    static HeartbeatTask* spInstance;
    static OsBSem sLock;

   OsConfigDb* mpConfig;
   int mReboot;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _HeartbeatTask_h_
