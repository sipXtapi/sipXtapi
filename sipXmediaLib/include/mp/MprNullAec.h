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
*
*  Typically AEC looks at the output to the speaker and subtracts that signal
*  in some form from the input from the mic to remove the echo.  So generically
*  AEC has two inputs (mic and speaker) and one output (mic - echo). If this was
*  done in a single resource this would create a loop in the process order of
*  the resources (a resource must get all its inputs to generate its outputs).
*
*  To avoid this loop, AEC is broken into two resources.  The MprAudioFrameBuffer
*  which simply stores references to the last MpBufs that went through it and 
*  makes it availabe via an accessor.  AEC uses the MprAudioFrameBuffer to get
*  the speaker output.  So the real AEC resource then has only one input (mic) and
*  one output (mic - echo).  It gets the speaker output indirectly from the
*  MprAudioFrameBuffer.
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
