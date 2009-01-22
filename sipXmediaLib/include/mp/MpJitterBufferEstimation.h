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

#ifndef _MpJitterBufferEstimation_h_
#define _MpJitterBufferEstimation_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <os/OsStatus.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
struct RtpHeader;

/**
*  @brief Abstract base class for Jitter Buffer Estimation (JBE) algorithms.
*/
class MpJitterBufferEstimation
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Destructor
   virtual ~MpJitterBufferEstimation() {};

     /// Initialize algorithm with given samplerate.
   virtual OsStatus init(int samplerate) = 0;
     /**
     *  Should be called before any other class methods.
     */

     /// Factory method for VAD algorithms creation.
   static MpJitterBufferEstimation *createJbe(const UtlString &algName = "");
     /**<
     *  @param[in] algName - name of JBE algorithm to use. Use empty string
     *             to get default algorithm.
     *
     *  @returns Method never returns NULL. If appropriate JBE algorithm is
     *           not found, default one is returned.
     */

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Update estimation with new received packet.
   virtual OsStatus update(const RtpHeader *rtp,
                           uint32_t cur_rtp_timestamp,
                           uint32_t cur_playback_time,
                           int32_t *hint) = 0;

     /// Set algorithm to be used by default.
   static void setDefaultAlgorithm(const UtlString& name);
     /**<
     *  Initially default algorithm is defined at compile time. Using this
     *  function you can change default algorithm at run-time or switch back to
     *  compile-time default.
     *
     *  @param[in] name - name of algorithm to use by default. Reverts to
     *             compile-time default if empty.
     */

     /// Prepare to process other unrelated audio stream.
   virtual void reset() = 0;
     /**<
     *  Only initialized algorithm can be reseted. Calling reset() should
     *  bring algorithm to the original state as it was right after init().
     *  So init() should NOT be called after reset().
     */

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

   static UtlString smDefaultAlgorithm; ///< Name of algorithm to be used by default.

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpJitterBufferEstimation_h_
