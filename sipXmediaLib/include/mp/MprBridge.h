//  
// Copyright (C) 2006-2012 SIPez LLC.  All rights reserved.
//
// Copyright (C) 2004-2008 SIPfoundry Inc.
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
#include <mp/MpBridgeAlgBase.h>
#include <os/OsIntTypes.h>

// DEFINES
#define TEST_PRINT_CONTRIBUTORS
#undef  TEST_PRINT_CONTRIBUTORS

//#define PRINT_CLIPPING_STATS
#define PRINT_CLIPPING_FREQUENCY 100

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
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
   
   /// Type of the bridge mix algorithm to use.
   enum AlgType
   {
      ALG_SIMPLE, ///< Simple O(n^2) algorithm (MpBridgeAlgSimple)
      ALG_LINEAR  ///< Linear O(n) algorithm (MpBridgeAlgLinear)
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MprBridge(const UtlString& rName,
             int maxInOutputs,
             UtlBoolean mixSilence=TRUE,
             AlgType algorithm=ALG_LINEAR);

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
                                   const MpBridgeGain gain[]);
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

     /// Send message to set mix weights for inputs to given output on bridge.
   static OsStatus setMixWeightsForOutput(const UtlString& namedResource, 
                                          OsMsgQ& fgQ,
                                          int bridgeOutputPort,
                                          int numWeights,
                                          const MpBridgeGain gains[]);
     /**<
     *  @see setMixWeightsForOutput(int,int,MpBridgeGain[]) for description.
     */

     /// Send message to set mix weights for one input to given number of outputs.
   OsStatus setMixWeightsForInput(int bridgeInputPort,
                                  int numWeights,
                                  const MpBridgeGain gain[]);
     /**<
     *  @see This function is full analog of setMixWeightsForOutput(), applied
     *       for one input to outputs instead of inputs to one output.
     */

     /// Send message to set mix weights for one input to given number of outputs.
   static OsStatus setMixWeightsForInput(const UtlString& namedResource, 
                                         OsMsgQ& fgQ,
                                         int bridgeInputPort,
                                         int numWeights,
                                         const MpBridgeGain gains[]);
     /**<
     *  @see setMixWeightsForOutput(int,int,MpBridgeGain[]) for description.
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

#ifdef TEST_PRINT_CONTRIBUTORS  // [
   MpContributorVector*  mpMixContributors;
   MpContributorVector** mpLastOutputContributors;
#endif // TEST_PRINT_CONTRIBUTORS ]

   AlgType mAlgType;              ///< Type of the bridge algorithm to use.
   MpBridgeAlgBase *mpBridgeAlg;  ///< Instance of algorithm, used to mix data.
   UtlBoolean mMixSilence;        ///< Should Bridge ignore or mix frames marked as silence?

#ifdef PRINT_CLIPPING_STATS
   int mClippedFramesCounted;
   int* mpOutputClippingCount;
#endif

     /// @brief Associates this resource with the indicated flow graph.
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  We use this overloaded method for initialization of some of our member
     *  variables, which depend on flowgraph's properties (like frame size).
     *
     *  @retval OS_SUCCESS - for now, this method always returns success
     */

     /// @brief Mix together inputs onto outputs according to mpGainMatrix matrix.
   UtlBoolean doMix(MpBufPtr inBufs[], int inBufsSize,
                    MpBufPtr outBufs[], int outBufsSize,
                    int samplesPerFrame);

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// Handle flowgraph messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Actually set mix weights for inputs to given output on bridge.
   virtual UtlBoolean handleSetMixWeightsForOutput(int bridgeOutputPort,
                                                   int numWeights,
                                                   const MpBridgeGain gain[]);
     /**<
     *  @see setMixWeightsForOutput() for explanation of parameters.
     */

     /// Actually set mix weights for one input to given number of outputs.
   virtual UtlBoolean handleSetMixWeightsForInput(int bridgeInputPort,
                                                  int numWeights,
                                                  const MpBridgeGain gain[]);
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
