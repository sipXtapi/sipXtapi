//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsPathLinux_h_
#define _OsPathLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsPathBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathBase;

//:OS generic path class.  Will massage any input string so separators are correct.
//:Also provided functions to 
class OsPathLinux : public OsPathBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsPathLinux();
     //:Default constructor

   OsPathLinux(const OsPathLinux& rOsPathLinux);
     //:Copy constructor

   virtual
   ~OsPathLinux();
     //:Destructor

   OsPathLinux(const UtlString& rPath);
     //: Copy contructor

   OsPathLinux(const char* pPath);
     //: Construct OsPathLinux from char*
    
   OsPathLinux(const UtlString& rVolume, const UtlString& rDirName, const UtlString& rFileName,
           const UtlString& rExtension);
     //: Forms a OsPathLinux from discrete parts

/* ============================ MANIPULATORS ============================== */

    OsPathLinux& operator=(const OsPathLinux& rhs);
      //:Assignment operator

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsPathLinux_h_


