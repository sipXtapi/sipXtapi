//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MiIntNotf_h_
#define _MiIntNotf_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "utl/UtlString.h"
#include "mi/MiNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message notification class used to communicate abstract integer value.
class MiIntNotf : public MiNotification
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MiIntNotf(NotfType msgType,
             const UtlString& sourceId, 
             int value,
             int connectionId = INVALID_CONNECTION_ID,
             int streamId = -1);

     /// Copy constructor
   MiIntNotf(const MiIntNotf& rNotf);

     /// Create a copy of this msg object (which may be of a derived type)
   virtual OsMsg* createCopy() const;

     /// Destructor
   virtual ~MiIntNotf();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Assignment operator
   MiIntNotf& operator=(const MiIntNotf& rhs);

     /// Set the value this notification reports.
   void setValue(int value);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Get the value this notification reports.
   int getValue() const;

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int mValue; ///< Reported value.
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MiIntNotf_h_
