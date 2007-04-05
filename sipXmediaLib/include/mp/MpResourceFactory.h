//  
// Copyright (C) 2006-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MpResourceFactory_h_
#define _MpResourceFactory_h_

// SYSTEM INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>

// APPLICATION INCLUDES
// DEFINES
#define DEFAULT_BRIDGE_RESOURCE_TYPE "MprBridge"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpResourceConstructor;
class MpResource;

/**
*  @brief MpResourceFactory is used to contruct resources by type name.
*
*  MpResourceConstructors are added to the MpResourceFactory and registered 
*  with a resource type name.  Resources are then constructed using the
*  MpResourceFactory.
*
*/
class MpResourceFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MpResourceFactory();

    /** Destructor
     */
    virtual ~MpResourceFactory();

/* ============================ MANIPULATORS ============================== */

    /// Add a constructor for a new resource type
    OsStatus addConstructor(MpResourceConstructor& resourceConstructor);

    /// Create a new resource of given type
    MpResource* newResource(const UtlString& resourceType, 
                            const UtlString& newResourceName) const;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


    /** Disabled copy constructor
     */
    MpResourceFactory(const MpResourceFactory& rMpResourceFactory);


    /** Disable assignment operator
     */
    MpResourceFactory& operator=(const MpResourceFactory& rhs);

    
    UtlHashMap mConstructors;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceFactory_h_
