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

#ifndef _MpAgcSimple_h_
#define _MpAgcSimple_h_

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
*  @brief Simple class for AGC algorithm.
*
*  @nosubgrouping
*/

class MpAgcSimple : public MpAgcBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

   const static UtlString name;  ///< Name of this AGC algorithm.

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   MpAgcSimple();

     /// @copydoc MpAgcBase::init
   OsStatus init(int samplerate);

     /// Destructor
   ~MpAgcSimple();

     /// Reset
   void reset();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// @copydoc MpAgcBase::processFrame
    OsStatus processFrame(const MpAudioSample* data, int count);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

     /// @copydoc MpAgcBase::getAmplitude
   OsStatus getAmplitude(MpAudioSample& amp, UtlBoolean &isClipped);

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   float mAdaptation;
   MpAudioSample mAmplitude;
   UtlBoolean mWasClipping;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpAgcSimple_h_
