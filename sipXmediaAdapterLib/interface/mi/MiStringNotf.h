//  
// Copyright (C) 2008-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Daniel Petrie <dpetrie AT SIPez DOT com>

#ifndef _MiStringNotf_h_
#define _MiStringNotf_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <mi/MiNotification.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message notification class used to communicate abstract integer value.
class MiStringNotf : public MiNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
    MiStringNotf(NotfType msgType,
                 const UtlString& sourceId, 
                 const UtlString& value,
                 int connectionId = INVALID_CONNECTION_ID,
                 int streamId = -1);

     /// Copy constructor
    MiStringNotf(const MiStringNotf& rNotf);

     /// Create a copy of this msg object (which may be of a derived type)
    virtual OsMsg* createCopy() const;

     /// Destructor
    virtual ~MiStringNotf();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
    MiStringNotf& operator=(const MiStringNotf& rhs);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Set the value this notification reports.
    void setValue(const UtlString& value);

     /// Get the value this notification reports.
    void getValue(UtlString& value) const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mValue; ///< Reported value.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiStringNotf_h_
