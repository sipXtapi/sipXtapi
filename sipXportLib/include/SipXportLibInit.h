// 
// Copyright (C) 2007 Jaroslav Libak
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

/**
*  You SHOULD include SipXportLibInit.h as the first include in .cpp file if it
*  contains a definition of a static variable that uses sipXportLib static
*  objects in its constructor or destructor. Including SipXportLibInit.h will
*  ensure, that all dependencies will be initialized before the custom static
*  object, and destructed after destruction of the custom object.
*  
*  @note If you need only part of sipXportLib static objects (e.g only Utl ones)
*        you may inlcude only specific initializers headers (e.g. UtlInit.h for
*        Utl).
*  @see os/OsProtectEventMgrInit.h
*  @see utl/UtlInit.h
*  @see os/OsNameDbInit.h
*/


#ifndef SipXportLibInit_h__
#define SipXportLibInit_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES

// Be sure to include all dependencies in the right order!
#include "os/OsProtectEventMgrInit.h"
#include "utl/UtlInit.h"
#include "os/OsNameDbInit.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

#endif // SipXportLibInit_h__
