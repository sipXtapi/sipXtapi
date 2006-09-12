//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsSharedLibMgrLinux_h_
#define _OsSharedLibMgrLinux_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSharedLibMgr.h>

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
class OsSharedLibMgrLinux : public OsSharedLibMgrBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
    friend class OsSharedLibMgrBase;

public:

/* ============================ CREATORS ================================== */

   virtual
   ~OsSharedLibMgrLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual OsStatus loadSharedLib(const char* libName);
   //: Loads the given shared library
   //!param: libName - name of library, may include absolute or relative path

   virtual OsStatus getSharedLibSymbol(const char* libName,
                              const char* symbolName,
                              void*& symbolAddress);
   //: Gets the address of a symbol in the shared lib
   //!param: (in) libName - name of library, may include absolute or relative path
   //!param: (in) symbolName - name of the variable or function exported in the shared lib
   //!param: (out) symbolAddress - the address of the function or variable

   virtual OsStatus unloadSharedLib(const char* libName);
   //: Not yet implemented

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsSharedLibMgrLinux();
     //:Default constructor disallowed, use getOsSharedLibMgr

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSharedLibMgrLinux(const OsSharedLibMgrLinux& rOsSharedLibMgrLinux);
     //:Copy constructor

   OsSharedLibMgrLinux& operator=(const OsSharedLibMgrLinux& rhs);
     //:Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsSharedLibMgrLinux_h_


