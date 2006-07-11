// $Id: //depot/OPENDEV/sipXphone/include/pinger/PingerConfig.h#2 $
// 
// Copyright (C) 2004 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2004 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PingerConfig_h_
#define _PingerConfig_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
extern UtlBoolean SipLineMgrSerialize(SipLineMgr& lineMgr, UtlBoolean bSave);
   //:Serialize all of the configuration settings to/from persistent storage
   //!param: (in) bSave - Controls whether data is being stored (TRUE) or 
   //        retrieved (FALSE).  
   //!returns: TRUE if any items are saved or restored, otherwise FALSE

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Pinger/Phone related configuration related functions and utilities
// This is intended as a container for all Phone product
// specific functions and utilities realted to configuration.
// PLEASE think twice before making a phone spscific utility.
// Try to generalize to make if more reusable.  Hoever if this
// is unavoidable, this is the place to put it.



/* ============================ INLINE METHODS ============================ */

#endif  // _PingerConfig_h_
