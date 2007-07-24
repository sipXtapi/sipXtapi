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

#ifndef _MprNullAec_h_
#define _MprNullAec_h_

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
class MprAudioFrameBuffer;

/**
*  @brief The MprNullAec resource a no-op AEC implementation intended to be a base calls for real AEC implementations..
*
*  This resource passes buffers from input 0 straight through to output 0.
*/
class MprNullAec : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MprNullAec(const UtlString& rName, 
              int samplesPerFrame, 
              int samplesPerSec,
              MprAudioFrameBuffer& MprAudioFrameBuffer);

     /// Destructor
   virtual
   ~MprNullAec();

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

   MprAudioFrameBuffer* mpOutputBufferResource;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprNullAec(const MprNullAec& rMprNullAec);

     /// Assignment operator (not implemented for this class)
   MprNullAec& operator=(const MprNullAec& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNullAec_h_
