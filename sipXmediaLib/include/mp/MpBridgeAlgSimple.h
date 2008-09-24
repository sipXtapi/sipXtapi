//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MpBridgeAlgSimple_h_
#define _MpBridgeAlgSimple_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBridgeAlgBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Brief description.
*
*  Long description here.
*  Even multiline.
*/
class MpBridgeAlgSimple : public MpBridgeAlgBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor.
   MpBridgeAlgSimple(int maxInputs, int maxOutputs, UtlBoolean mixSilence,
                     int samplesPerFrame);

     /// Destructor.
   ~MpBridgeAlgSimple();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// @copydoc MpBridgeAlgBase::doMix()
   UtlBoolean doMix(MpBufPtr inBufs[], int inBufsSize,
                    MpBufPtr outBufs[], int outBufsSize,
                    int samplesPerFrame);

     /// @copydoc MpBridgeAlgBase::setGainMatrixValue()
   void setGainMatrixValue(int column, int row, MpBridgeGain val);

     /// @copydoc MpBridgeAlgBase::setGainMatrixRow()
   void setGainMatrixRow(int row, int numValues, const MpBridgeGain val[]);

     /// @copydoc MpBridgeAlgBase::setGainMatrixColumn()
   void setGainMatrixColumn(int column, int numValues, const MpBridgeGain val[]);

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

   MpBridgeGain*  mpGainMatrix;        ///< mMaxOutputs x mMaxInputs array
                    ///< of inputs to outputs gains.
   MpBridgeAccum* mpMixAccumulator;    ///< Accumulator to store sum of all inputs.
                    ///< have size of mSamplesPerFrame. Used in doMix() only.

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpBridgeAlgSimple_h_
