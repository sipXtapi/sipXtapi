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

#ifndef _MprAudioFrameBuffer_h_
#define _MprAudioFrameBuffer_h_

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
*  @brief The MprAudioFrameBuffer resource buffers frames of audio that pass through to be retrieved later.
*
*  This resource always passes input 0 through to output 0, but it keeps a
*  reference of the buffer passed through for a configured length of time 
*  (frame periods).  No latency is added to the input.  References are just 
*  kept for the input buffers which can be accessed via the getFrame method.
*/
class MprAudioFrameBuffer : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MprAudioFrameBuffer(const UtlString& rName, 
                       int samplesPerFrame, 
                       int samplesPerSec,
                       int numFramesHistory);

     /// Destructor
   virtual
   ~MprAudioFrameBuffer();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   OsStatus getFrame(int pastFramesIndex, MpBufPtr& frameBuffer);

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
   MprAudioFrameBuffer(const MprAudioFrameBuffer& rMprAudioFrameBuffer);

     /// Assignment operator (not implemented for this class)
   MprAudioFrameBuffer& operator=(const MprAudioFrameBuffer& rhs);

   int mFrameCount;
   int mNumBufferFrames;
   MpBufPtr* mpBufferedFrameArray;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprAudioFrameBuffer_h_
