//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie <dpetrie AT SIPez DOT com>

#ifndef _MprNull_h_
#define _MprNull_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "Null" media processing resource is ground or /dev/null equivalent.
*
*  This resource releases all input buffers and outputs null buffers.
*/
class MprNull : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MprNull(const UtlString& rName, 
           int maxInOutputs,
           int samplesPerFrame, 
           int samplesPerSec);

     /// Destructor
   virtual
   ~MprNull();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
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

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprNull(const MprNull& rMprNull);

     /// Assignment operator (not implemented for this class)
   MprNull& operator=(const MprNull& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNull_h_
