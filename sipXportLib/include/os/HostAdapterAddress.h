//
// Copyright (C) 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


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
