//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprBridge_h_
#define _MprBridge_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpAudioResource.h>
#include <mp/MpFlowGraphMsg.h>
#include <mp/MpDspUtils.h>
#include <os/OsIntTypes.h>

// DEFINES
#define TEST_PRINT_CONTRIBUTORS
#undef  TEST_PRINT_CONTRIBUTORS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
#ifdef MP_FIXED_POINT // [
   typedef int_fast16_t MpBridgeGain;
#else  // MP_FIXED_POINT ][
   typedef float MpBridgeGain;
#endif // MP_FIXED_POINT ]

// MACROS

// Use Q5.10 (signed) values when fixde point is enabled.
#define MP_BRIDGE_INT_LENGTH          INT16_C(5)
#define MP_BRIDGE_FRAC_LENGTH         INT16_C(10)
#define MPF_BRIDGE_FLOAT(num)         ((MpBridgeGain)MPF_FLOAT((num), MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))
#define MPF_BRIDGE_MAX                ((MpBridgeGain)MPF_MAX(MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))
#define MPF_BRIDGE_STEP               ((MpBridgeGain)MPF_STEP(MP_BRIDGE_INT_LENGTH, MP_BRIDGE_FRAC_LENGTH))

#define MP_BRIDGE_GAIN_UNDEFINED      INT_FAST16_MAX
#define MP_BRIDGE_GAIN_MAX            MPF_BRIDGE_MAX
#define MP_BRIDGE_GAIN_MIN            MPF_BRIDGE_FLOAT(0.0f)
#define MP_BRIDGE_GAIN_STEP           MPF_BRIDGE_STEP
#define MP_BRIDGE_GAIN_MUTED          MP_BRIDGE_GAIN_MIN
#define MP_BRIDGE_GAIN_PASSTHROUGH    MPF_BRIDGE_FLOAT(1.0f)

// FORWARD DECLARATIONS
#ifdef TEST_PRINT_CONTRIBUTORS  // [
class MpContributorVector;
#endif // TEST_PRINT_CONTRIBUTORS ]

/**
*  @brief The conference bridge resource.
*
*  Local input and output should be connected to the first input and output
*  ports. Remote connections will be binded to the second input/output ports,
*  third input/output ports, and so on.
*
*  <H3>Enabled behaviour</H3>
*  Mix together local and remote inputs onto outputs, according to mix matrix.
*  By deafault matrix is defined so that no output receive its own input.
*
*  <H3>Disabled behaviour</H3>
*  Drop all input data, do not produce any data on output.
*/
class MprBridge : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MprBridge(const UtlString& rName,
             int maxInOutputs, 
             int samplesPerFrame, 
             int samplesPerSec);

     /// Destructor
   virtual
   ~MprBridge();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Send message to set mix weights for inputs to given output on bridge.
   OsStatus setMixWeightsForOutput(int bridgeOutputPort,
                                   int numWeights,
                                   MpBridgeGain gain[]);
     /**<
     *  Set the mix weights for the inputs (indices 0 to \p numWeights) going 
     *  to the output port: \p bridgeOutputPort of the named bridge.
     *
     *  @param[in] bridgeOutputPort - index (starts at 0) of the output port to
     *             which the input weights are to be set.
     *  @param[in] numWeights - number of elements in \p gain array. If less,
     *             then only first \p numWeights elements would be set.
     *  @param[in] gain - the gain value or multiplier to apply to the
     *             corresponding input.
     *             * MP_BRIDGE_GAIN_PASSTHROUGH will add the unchanged input
     *               to the output.
     *             * MP_BRIDGE_GAIN_UNDEFINED will use the existing value of
     *               gain for the input.
     *
     *  @note If gain is not set to MP_BRIDGE_GAIN_MUTED for the input
     *        corresponding to the output port, there will be echo.
     *
     *  @see setMixWeightsForInput()
     */

     /// Send message to set mix weights for one input to given number of outputs.
   OsStatus setMixWeightsForInput(int bridgeInputPort,
                                  int numWeights,
                                  MpBridgeGain gain[]);
     /**<
     *  @see This function is full analog of setMixWeightsForOutput(), applied
     *       for one input to outputs instead of inputs to one output.
     */

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

   typedef enum
   {
      SET_WEIGHTS_FOR_INPUT = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_WEIGHTS_FOR_OUTPUT
   } AddlMsgTypes;


#ifdef MP_FIXED_POINT // [
   typedef int32_t MpBridgeAccum;
#else  // MP_FIXED_POINT ][
   typedef float MpBridgeAccum;
#endif // MP_FIXED_POINT ]

#ifdef TEST_PRINT_CONTRIBUTORS  // [
   MpContributorVector*  mpMixContributors;
   MpContributorVector** mpLastOutputContributors;
#endif // TEST_PRINT_CONTRIBUTORS ]

   MpBridgeGain*  mpGainMatrix;        ///< mMaxOutputs x mMaxInputs array
   MpBridgeAccum* mpMixAccumulator;    ///< Accumulator to store sum of all inputs.
                    ///< have size of mSamplesPerFrame. Used in doMix() only.
   MpBridgeAccum* mpGainedInputs;      ///< 2D array to store inputs data with
                    ///< applied gain. It have mMaxInputs rows of
                    ///< mSamplesPerFrame samples.  Used in doMix() only.
   bool*          mpIsInputActive;     ///< Store is input active or not for
                    ///< optimization reasons. Used in doMix() only.

     /// @brief Mix together inputs onto outputs according to mpGainMatrix matrix.
   UtlBoolean doMix(MpBufPtr inBufs[], int inBufsSize,
                    MpBufPtr outBufs[], int outBufsSize,
                    int samplesPerFrame);

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Handle flowgraph messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Actually set mix weights for inputs to given output on bridge.
   virtual UtlBoolean handleSetMixWeightsForOutput(int bridgeOutputPort,
                                                   int numWeights,
                                                   MpBridgeGain gain[]);
     /**<
     *  @see setMixWeightsForOutput() for explanation of parameters.
     */

     /// Actually set mix weights for one input to given number of outputs.
   virtual UtlBoolean handleSetMixWeightsForInput(int bridgeInputPort,
                                                  int numWeights,
                                                  MpBridgeGain gain[]);
     /**<
     *  @see setMixWeightsForOutput() for explanation of parameters.
     */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprBridge(const MprBridge& rMprBridge);

     /// Assignment operator (not implemented for this class)
   MprBridge& operator=(const MprBridge& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprBridge_h_
