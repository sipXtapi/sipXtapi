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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpResourceFactory.h>
#include <mp/MpResourceConstructor.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpResourceFactory::MpResourceFactory()
{
}

// Destructor
MpResourceFactory::~MpResourceFactory()
{
    mConstructors.destroyAll();
}

/* ============================ MANIPULATORS ============================== */
OsStatus MpResourceFactory::addConstructor(MpResourceConstructor& resourceConstructor)
{
    OsStatus result;
    if(mConstructors.find(&resourceConstructor))
    {
        // A constructor already exists for this resource type
        result = OS_NAME_IN_USE;
    }

    else
    {
        result = OS_SUCCESS;
        mConstructors.insert(&resourceConstructor);
    }

    return(result);
}

    /// Create a new resource of given type
OsStatus MpResourceFactory::newResource(const UtlString& resourceType,
                                        const UtlString& newResourceName,
                                        int maxResourcesToCreate,
                                        int& numResourcesCreated,
                                        MpResource* resourcesCreated[]) const
{
    MpResourceConstructor* constructor = 
        (MpResourceConstructor*) mConstructors.find(&resourceType);

    return(constructor ? 
           constructor->newResource(newResourceName,
                                    maxResourcesToCreate,
                                    numResourcesCreated,
                                    resourcesCreated) : 
          OS_NOT_FOUND);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean MpResourceFactory::constructorExists(const UtlString& resourceType)
{
    MpResourceConstructor* constructor = 
        (MpResourceConstructor*) mConstructors.find(&resourceType);

    return(constructor != NULL);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

