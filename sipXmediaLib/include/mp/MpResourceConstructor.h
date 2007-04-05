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

#ifndef _MpResourceConstructor_h_
#define _MpResourceConstructor_h_

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
*  @brief MpResourceConstructor is used to contruct a spcific resource type
*
*  MpResourceConstructors is an abstract class.  Derived types construct a
*  specific type of MpResource.
*
*/
class MpResourceConstructor : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /** Constructor
     */
    MpResourceConstructor(const UtlString& resourceType,
                          int minInputs = 1,
                          int maxInputs = 1,
                          int minOutputs = 1,
                          int maxOutputs = 1) :
      UtlString(resourceType),
      mMinInputs(minInputs),
      mMaxInputs(maxInputs),
      mMinOutputs(minOutputs),
      mMaxOutputs(maxOutputs)
    {
    };

    /** Destructor
     */
    virtual ~MpResourceConstructor(){};

/* ============================ MANIPULATORS ============================== */

    /// Create a new resource
    virtual MpResource* newResource(const UtlString& resourceName) = 0;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    int mMinInputs;
    int mMaxInputs;
    int mMinOutputs;
    int mMaxOutputs;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Disable default constructor
    MpResourceConstructor();

    /** Disabled copy constructor
     */
    MpResourceConstructor(const MpResourceConstructor& rMpResourceConstructor);


    /** Disable assignment operator
     */
    MpResourceConstructor& operator=(const MpResourceConstructor& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResourceConstructor_h_
