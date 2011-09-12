//
// Copyright (C) 2006-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlCopyableContainable_h_
#define _UtlCopyableContainable_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlSList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * UtlCopyableContainable is a containable that must implement a clone method, that is 
 * used by UtlCopyableSList in order to assign/copy list elements on list copy.
 *
 * Derived classes should also implement a copy constructor and an equals 
 * operator.
 *
 * Sample clone implementation:
 * SampleClass* clone() const { return new SampleClass(*this); }
 *
 */
class UtlCopyableContainable : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   /// Make an off the heap copy of this object
   virtual UtlCopyableContainable* clone() const = 0;

/* ============================ CREATORS ================================== */

UtlCopyableContainable();

virtual ~UtlCopyableContainable();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

    /// @copydoc UtlContainable::getContainableType
    virtual UtlContainableType getContainableType() const;

/* ============================ INQUIRY =================================== */
   
    /// @copydoc UtlContainable::isInstanceOf
    virtual UtlBoolean isInstanceOf(const UtlContainableType type) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

} ;

/* ============================ INLINE METHODS ============================ */\

#endif    // _UtlCopyableContainable_h_

