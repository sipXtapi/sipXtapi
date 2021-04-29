//
// Copyright (C) 2007-2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpALSADeviceWrapperContainer_h_
#define _MpALSADeviceWrapperContainer_h_


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
class MpAlsa;

class MpAlsaContainer : protected UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

   MpAlsaContainer();

   ~MpAlsaContainer();
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// @brief Create new wrapper for name haven't founded in container. 
   MpAlsa* getALSADeviceWrapper(const UtlString& ossdev);

     /// @brief Excluding wrapper from container.
   void excludeFromContainer(MpAlsa* pDev);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

     /// @brief Creating wrapper and return if it wasn't create yet.
   static
   MpAlsaContainer* getContainer();

     /// @brief Releasing wrapper if it no longer needed (no references).
   static
   void releaseContainer(MpAlsaContainer* pCont);

     /// @brief Excluding wrapper from container (static version).
   static
   UtlBoolean excludeWrapperFromContainer(MpAlsa* pDev);

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   static MpAlsaContainer* mpCont; ///< Pointer to allocated class
   static int refCount; ///< Number of all MpALSADeviceWrapperContainer members

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex mMutex;      ///< Internal synchronization mutex
};

/* ============================ INLINE METHODS ============================ */

#endif // _MpALSADeviceWrapperContainer_h_
