//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef HostAdapterAddress_h
#define HostAdapterAddress_h

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * Simple class definition for an object that contains
 */ 
class HostAdapterAddress
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Constructor accepting an optional default value.
     */
    HostAdapterAddress(char* szAdapter, char* szAddress) :
        mAdapter(szAdapter), 
        mAddress(szAddress)
        {
        }
      
    /**
     * Destructor
     */
    virtual ~HostAdapterAddress()
    {
    }

    UtlString mAdapter;
    UtlString mAddress;
    
/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    
};

#endif
