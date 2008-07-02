//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander.Chemeris AT SIPez DOT com>

#ifndef _MpJbeFixed_h_
#define _MpJbeFixed_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpJitterBufferEstimation.h"
#include <os/OsIntTypes.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Estimation algorithm for simple fixed length jitter buffer.
*/
class MpJbeFixed : public MpJitterBufferEstimation
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
   static const char *name; ///< Name of this JBE algorithm for use in
                            ///< MpJitterBufferEstimation::createJbe().

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   MpJbeFixed();

     /// Destructor
   virtual ~MpJbeFixed();

     /// Initialize with given samplerate.
   virtual OsStatus init(int samplerate);

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Update estimation with new received packet.
   virtual OsStatus update(const RtpHeader *rtp,
                           uint32_t cur_rtp_timestamp,
                           uint32_t cur_playback_time,
                           int32_t *hint);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{


//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   int      mSamplerate;
   int32_t  mDelay;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJbeFixed_h_
