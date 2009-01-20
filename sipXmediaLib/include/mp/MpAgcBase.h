//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Sergey Kostanbaev <Sergey DOT Kostanbaev AT sipez DOT com>

#ifndef _MpAgcBase_h_
#define _MpAgcBase_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "mp/MpTypes.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Base class for AGC algorithm.
*
*  @nosubgrouping
*/
class MpAgcBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Factory method for VAD algorithms creation.
   static MpAgcBase *createAgc(const UtlString &name = "");
     /**<
     *  @param[in] name - name of AGC algorithm to use. Use empty string
     *             to get default algorithm.
     *
     *  @returns Method never returns NULL. If appropriate AGC algorithm is
     *           not found, default one is returned.
     */

     /// Initialize AGC with given sample rate.
   virtual OsStatus init(int samplerate) = 0;
     /**<
     *  Should be called before any other class methods.
     */

     /// Destructor
   virtual ~MpAgcBase() {};

     /// Reset
   virtual void reset() = 0;

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Calculate amplitude of the frame.
   virtual OsStatus processFrame(const MpAudioSample* data, int count) = 0;

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

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// Get amplitude of the last processed frame.
   virtual OsStatus getAmplitude(MpAudioSample& amp, UtlBoolean &isClipped) = 0;

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

#endif  // _MpAgcBase_h_
