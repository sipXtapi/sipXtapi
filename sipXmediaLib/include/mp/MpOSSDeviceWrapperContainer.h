//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpOSSDeviceWrapperContainer_h_
#define _MpOSSDeviceWrapperContainer_h_


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpOSSDeviceWrapper;

class MpOSSDeviceWrapperContainer : protected UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpOSSDeviceWrapperContainer();

   ~MpOSSDeviceWrapperContainer();
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   MpOSSDeviceWrapper* getOSSDeviceWrapper(const UtlString& ossdev);

   void excludeFromContainer(MpOSSDeviceWrapper* pDev);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex mMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif // _MpOSSDeviceWrapperContainer_h_
