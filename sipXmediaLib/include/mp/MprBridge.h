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

#define TEST_PRINT_MIXING
#undef  TEST_PRINT_MIXING

#define TEST_PRINT_MIXING_BINARY
#undef  TEST_PRINT_MIXING_BINARY

#define LINEAR_COMPLEXITY_BRIDGE
//#undef  LINEAR_COMPLEXITY_BRIDGE

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
#ifdef MP_FIXED_POINT // [
   typedef int16_t MpBridgeGain;
#else  // MP_FIXED_POINT ][
   typedef float MpBridgeGain;
#endif // MP_FIXED_POINT ]

// MACROS

// Use Q5.10 (signed) values when fixed point is enabled.
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
             int samplesPerSec,
             UtlBoolean mixSilence=TRUE);

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
                    ///< of inputs to outputs gains.

/* ============================ Extended inputs =========================== */
///@name Extended inputs
//@{

   struct ExtendedInputs
   {
      ExtendedInputs()
      : mExtendedInputsNum(0)
      , mInputsNum(0)
      , mOutputsNum(0)
      , mpExtendedInputsMap(NULL)
      , mpExtendedInputsInfo(NULL)
      {};

      ~ExtendedInputs()
      {
         delete[] mpExtendedInputsMap;
         delete[] mpExtendedInputsInfo;
      }

        /// @brief Initialize extended inputs, assuming simple bridge matrix
        /// case (inversed unity matrix).
      inline void init_simple(int numInputs, int numOutputs)
      {
         // Init array sizes
         mInputsNum = numInputs;
         mOutputsNum = numOutputs;
         assert(mInputsNum == mOutputsNum);

         // Each input could not be replicated more then mOutputsNum times.
         mExtendedInputsNum = mInputsNum*mOutputsNum;

         // Allocate arrays
         mpExtendedInputsMap = new int[mInputsNum*mOutputsNum];
         mpExtendedInputsInfo = new ExtendedInputInfo[mExtendedInputsNum];

         // Fill arrays with data
         int *pTmpInputsMapPtr = mpExtendedInputsMap;
         int input;
         for (input=0; input<mInputsNum; input++)
         {
            // Fill mpExtendedInputsMap row
            int output;
            for (output=0; output<input; output++)
            {
               *pTmpInputsMapPtr = input;
               pTmpInputsMapPtr++;
            }
            // This one is muted
            *pTmpInputsMapPtr = -1;
            pTmpInputsMapPtr++;
            for (output=input+1; output<mOutputsNum; output++)
            {
               *pTmpInputsMapPtr = input;
               pTmpInputsMapPtr++;
            }
         }

         // Fill mpExtendedInputsInfo.
         for (input=0; input<mInputsNum; input++)
         {
            mpExtendedInputsInfo[input].mInput = input;
            mpExtendedInputsInfo[input].mGain = MP_BRIDGE_GAIN_PASSTHROUGH;
            mpExtendedInputsInfo[input].mRefCounter = mOutputsNum-1;
         }
         for (input=mInputsNum; input<mExtendedInputsNum; input++)
         {
            mpExtendedInputsInfo[input].mInput = -1;
            mpExtendedInputsInfo[input].mGain = MP_BRIDGE_GAIN_MUTED;
            mpExtendedInputsInfo[input].mRefCounter = 0;
         }
      }

      inline void setGain(int input, int output, MpBridgeGain gain)
      {
         assert(input < mInputsNum && output < mOutputsNum);

         int extendedInput = mpExtendedInputsMap[input*mOutputsNum + output];

         if (extendedInput >= 0)
         {
            assert(mpExtendedInputsInfo[extendedInput].mRefCounter > 0);
            mpExtendedInputsInfo[extendedInput].mRefCounter--;
         }

         if (gain != MP_BRIDGE_GAIN_MUTED)
         {
            extendedInput = addExtendedInput(input, gain);
            assert(extendedInput >= 0);

            mpExtendedInputsMap[input*mOutputsNum + output] = extendedInput;
         } 
         else
         {
            mpExtendedInputsMap[input*mOutputsNum + output] = -1;
         }
      }

      inline int searchForExtendedInput(int input, MpBridgeGain gain) const
      {
         for (int i=0; i<mExtendedInputsNum; i++)
         {
            if (  mpExtendedInputsInfo[i].mInput == input
               && mpExtendedInputsInfo[i].mGain == gain)
            {
               // Found it.
               return i;
            }
         }

         // Have not found it.
         return -1;
      }

      inline int addExtendedInput(int input, MpBridgeGain gain)
      {
         int lastEmptySlot=-1;
         
         // Look for existing extended input and for first empty slot (just
         // in case)
         int i;
         for (i=0; i<mExtendedInputsNum; i++)
         {
            if (  mpExtendedInputsInfo[i].mInput == input
               && mpExtendedInputsInfo[i].mGain == gain)
            {
               // Found existing extended input.
               mpExtendedInputsInfo[i].mRefCounter++;
               return i;
            }
            if (mpExtendedInputsInfo[i].mRefCounter == 0)
            {
               // Found it.
               lastEmptySlot = i;
               break;
            }
         }
         // If we still have not found existing extended output, continue on.
         // (seems we found empty slot, though)
         for (; i<mExtendedInputsNum; i++)
         {
            if (  mpExtendedInputsInfo[i].mInput == input
               && mpExtendedInputsInfo[i].mGain == gain)
            {
               // Found existing extended input.
               mpExtendedInputsInfo[i].mRefCounter++;
               return i;
            }
         }
         // At this point existing extended input is not found, so we should
         // have empty slot to add new one.
         assert(lastEmptySlot>=0);

         // Add new extended input.
         mpExtendedInputsInfo[lastEmptySlot].mRefCounter = 1;
         mpExtendedInputsInfo[lastEmptySlot].mInput = input;
         mpExtendedInputsInfo[lastEmptySlot].mGain = gain;

         // Have not found it.
         return lastEmptySlot;
      }

      inline int getExtendedInputsNum() const
      {
         return mExtendedInputsNum;
      }

      inline int getOrigin(int extendedInput) const
      {
         return mpExtendedInputsInfo[extendedInput].mInput;
      }

      inline int getGain(int extendedInput) const
      {
         return mpExtendedInputsInfo[extendedInput].mGain;
      }

      inline bool isNotMuted(int extendedInput) const
      {
         return (mpExtendedInputsInfo[extendedInput].mRefCounter > 0) &&
                (mpExtendedInputsInfo[extendedInput].mGain != MP_BRIDGE_GAIN_MUTED);
      }

      inline int getExtendedInput(int origInput, int output) const
      {
         return mpExtendedInputsMap[origInput*mOutputsNum + output];
      }

   protected:

      struct ExtendedInputInfo
      {
         int          mInput; ///< Index of original input for this extended input.
         MpBridgeGain mGain;  ///< Gain applied to this extended input.
         int          mRefCounter; ///< Number of original inputs, referencing this
                              ///< extended input.
      };

      int            mExtendedInputsNum;  ///< Size of mpExtendedInputsInfo array.
      int            mInputsNum;          ///< Number of outputs.
      int            mOutputsNum;         ///< Number of outputs.
      int*           mpExtendedInputsMap; ///< This array is a twin of
                       ///< mpGainMatrix. Each element is an index of
                       ///< corresponding extended input (in mpExtendedInputs
                       ///< array). Have size of mInputsNum x mOutputsNum.
      ExtendedInputInfo* mpExtendedInputsInfo; ///< Array with size of
                       ///< mExtendedInputsNum, storing info about extended
                       ///< inputs.
   };

   ExtendedInputs mExtendedInputs;     ///< Storage for all extended inputs data.

//@}

/* ============================= Active inputs ============================ */
///@name Active inputs
//@{

   int            mActiveInputsListSize;  ///< Length of mpActiveInputsList array.
   int*           mpActiveInputsList;     ///< Store the list of active extended
                    ///< inputs. Used in doMix() only.

//@}

/* ============================== Mix Engine ============================== */
///@name Mix Engine
//@{

   struct MixAction 
   {
      enum Type
      {
         DO_MIX,
         COPY_TO_OUTPUT,
         COPY_FROM_INPUT,
         NO_OPERATION
      };

      Type mType;
      int  mSrc1;
      int  mSrc2;
      int  mDst;
   };

   int            mMixActionsStackLength;
   int            mMixActionsStackTop;
   MixAction*     mpMixActionsStack;
   int            mMixDataStackStep;
   int            mMixDataStackLength;
   MpBridgeAccum* mpMixDataStackTop;
   MpBridgeAccum* mpMixDataStack;
   MpAudioBuf::SpeechType* mpMixDataSpeechType; ///< Speech type of data frames in mpMixDataStack
   int            mMixDataInfoStackStep;
   int            mMixDataInfoStackLength;
   int*           mpMixDataInfoStackTop;
   int*           mpMixDataInfoStack;
   int            mMixDataInfoProcessedStackLength;
   int            mMixDataInfoProcessedStackTop;
   int*           mpMixDataInfoProcessedStack;

#ifdef TEST_PRINT_MIXING // [
#ifdef TEST_PRINT_MIXING_BINARY // [
#  define PRINT_DATA_SET(offset) \
   for (int i=0; i<mMixDataInfoStackStep; i++) \
   { \
      printf(" %d", mpMixDataInfoStack[mMixDataInfoStackStep*offset + i]); \
   }
#  define PRINT_DATA_SET_TOP() \
   for (int i=0; i<mMixDataInfoStackStep; i++) \
   { \
      printf(" %d", mpMixDataInfoStackTop[i]); \
   }
#else // TEST_PRINT_MIXING_BINARY ][
#  define PRINT_DATA_SET(offset) \
   for (int i=0; i<mMixDataInfoStackStep; i++) \
   { \
      if (mpMixDataInfoStack[mMixDataInfoStackStep*offset + i] > 0) \
      { \
         printf(" %d", i); \
      } \
   }
#  define PRINT_DATA_SET_TOP() \
   for (int i=0; i<mMixDataInfoStackStep; i++) \
   { \
      if (mpMixDataInfoStackTop[i] > 0) \
      { \
         printf(" %d", i); \
      } \
   }
#endif //TEST_PRINT_MIXING_BINARY ]
#  define PRINT_TOP_MIX_DATA() \
   { \
      printf("pushed data %2d:           [", mMixDataInfoProcessedStackTop); \
      PRINT_DATA_SET_TOP() \
      printf("]\n"); \
   }
#else  // TEST_PRINT_MIXING ][
#  define PRINT_TOP_MIX_DATA()
#endif // TEST_PRINT_MIXING ]

   inline void initMixActions()
   {
      mMixActionsStackTop = 0;
   }

   inline void pushMixAction(MixAction::Type type, int src1, int src2, int dst)
   {
      assert(mMixActionsStackTop<mMixActionsStackLength);
      mpMixActionsStack[mMixActionsStackTop].mType = type;
      mpMixActionsStack[mMixActionsStackTop].mSrc1 = src1;
      mpMixActionsStack[mMixActionsStackTop].mSrc2 = src2;
      mpMixActionsStack[mMixActionsStackTop].mDst = dst;
      mMixActionsStackTop++;
   }

   inline void pushMixActionCopyToOutput(int src, int dst)
   {
#ifdef TEST_PRINT_MIXING // [
      printf("COPY_TO_OUTPUT:  %2d -> %2d [", src, dst);
      PRINT_DATA_SET(src);
      printf("]\n");
#endif // TEST_PRINT_MIXING ]
      pushMixAction(MixAction::COPY_TO_OUTPUT, src, -1, dst);
   }

   inline void pushMixActionCopyFromInput(int src, int dst)
   {
#ifdef TEST_PRINT_MIXING // [
      printf("COPY_FROM_INPUT: %2d <- %2d [", dst, src);
      PRINT_DATA_SET(dst);
      printf("]\n");
#endif // TEST_PRINT_MIXING ]
      // No real need to do this. Enable if you want debug.
      //mpMixDataInfoProcessedStack[src] = mMixActionsStackTop;
      pushMixAction(MixAction::COPY_FROM_INPUT, src, -1, dst);
   }

   inline void pushMixActionMix(int src1, int src2, int dst)
   {
#ifdef TEST_PRINT_MIXING // [
      printf("DO_MIX:     %2d + %2d -> %2d [", src1, src2, dst);
      PRINT_DATA_SET(src1);
      printf("] + [");
      PRINT_DATA_SET(src2);
      printf("] -> [");
      PRINT_DATA_SET(dst);
      printf("]\n");
#endif // TEST_PRINT_MIXING ]
      mpMixDataInfoProcessedStack[dst] = mMixActionsStackTop;
      pushMixAction(MixAction::DO_MIX, src1, src2, dst);
   }

   inline void moveTopMixActionMix(int dst)
   {
      int action = mpMixDataInfoProcessedStack[dst];
#ifdef TEST_PRINT_MIXING // [
      printf("pull:       %2d + %2d -> %2d\n",
             mpMixActionsStack[action].mSrc1,
             mpMixActionsStack[action].mSrc2,
             mpMixActionsStack[action].mDst);
#endif // TEST_PRINT_MIXING ]
      mpMixDataInfoProcessedStack[dst] = mMixActionsStackTop;
      pushMixAction(MixAction::DO_MIX,
                    mpMixActionsStack[action].mSrc1,
                    mpMixActionsStack[action].mSrc2,
                    mpMixActionsStack[action].mDst);
      mpMixActionsStack[action].mDst = MixAction::NO_OPERATION;
   }

   inline void initMixDataInfoStack(int step)
   {
      mpMixDataInfoStackTop = mpMixDataInfoStack;
      mMixDataInfoStackStep = step;
      mMixDataInfoProcessedStackTop = 0;
   }

   inline void initMixDataStack()
   {
      mpMixDataStackTop = mpMixDataStack;
   }

   inline bool searchClosestUnprocessedData(int  *pDataInfoStackPtr,
                                            int  *pStartDataInfoStackPtr,
                                            int   startDataInfoStackPos,
                                            int *&pFoundDataInfoStackPtr,
                                            int  &foundDataInfoStackPos)
   {
      int *pTmpPtr;
      int tmpPos;
      int foundCommonDataBitCounter = 0;
      int foundFirstDataDiffBitCounter  = 0;
      int foundSecondDataDiffBitCounter = 0;

      pFoundDataInfoStackPtr = pStartDataInfoStackPtr;
      foundDataInfoStackPos = startDataInfoStackPos;
      for (pTmpPtr = pStartDataInfoStackPtr, tmpPos=startDataInfoStackPos;
           pTmpPtr < mpMixDataInfoStackTop;
           pTmpPtr += mMixDataInfoStackStep, tmpPos++)
      {
         // Compare data sets if not processed yet and not equal to original
         // data set.
         if (mpMixDataInfoProcessedStack[tmpPos]<0 &&
             (pDataInfoStackPtr != pTmpPtr))
         {
            int  commonDataBitCounter = 0;
            int  firstDataDiffBitCounter  = 0;
            int  secondDataDiffBitCounter = 0;

            // Calculate common part, first only part and second only part.
            for (int i=0; i<mMixDataInfoStackStep; i++)
            {
               commonDataBitCounter     +=  pDataInfoStackPtr[i] &  pTmpPtr[i];
               firstDataDiffBitCounter  +=  pDataInfoStackPtr[i] & !pTmpPtr[i];
               secondDataDiffBitCounter += !pDataInfoStackPtr[i] &  pTmpPtr[i];
            }

            // Is this pair better then previous?
            if (commonDataBitCounter > foundCommonDataBitCounter &&
                firstDataDiffBitCounter < commonDataBitCounter &&
                secondDataDiffBitCounter < commonDataBitCounter)
            {
               pFoundDataInfoStackPtr = pTmpPtr;
               foundDataInfoStackPos = tmpPos;
               foundCommonDataBitCounter = commonDataBitCounter;
               foundFirstDataDiffBitCounter  = firstDataDiffBitCounter;
               foundSecondDataDiffBitCounter = secondDataDiffBitCounter;

               // Stop processing if close enough data set found.
               if ((foundFirstDataDiffBitCounter < 2) &&
                   (foundSecondDataDiffBitCounter < 2))
               {
                  break;
               }
            }
         }
      }

      return (foundCommonDataBitCounter > 0);
   }

   inline void searchExistingData(int  *pDataInfoStackPtr,
                                  int  &foundDataInfoStackPos)
   {
      int *pTmpPtr;
      int tmpPos;

      for (pTmpPtr = mpMixDataInfoStack, tmpPos=0;
           ;
           pTmpPtr += mMixDataInfoStackStep, tmpPos++)
      {
         int input=0;
         while (input<mMixDataInfoStackStep && 
                pDataInfoStackPtr[input] == pTmpPtr[input])
         {
            input++;
         }
         if (input == mMixDataInfoStackStep)
         {
            foundDataInfoStackPos = tmpPos;
            return;
         }
      }
   }

   inline void searchExistingUnprocessedData(int  *pDataInfoStackPtr,
                                             int  &foundDataInfoStackPos,
                                             int   ignoreDataInfoStackLevel)
   {
      int *pTmpPtr;
      int tmpPos;

      for (pTmpPtr = mpMixDataInfoStack, tmpPos=0;
           ;
           pTmpPtr += mMixDataInfoStackStep, tmpPos++)
      {
         if (mpMixDataInfoProcessedStack[tmpPos] < 0 ||
             tmpPos < ignoreDataInfoStackLevel)
         {
            // This is unprocessed data. Try it.

            int input=0;
            while (input<mMixDataInfoStackStep && 
                   pDataInfoStackPtr[input] == pTmpPtr[input])
            {
               input++;
            }
            if (input == mMixDataInfoStackStep)
            {
               // Found! Return value.
               foundDataInfoStackPos = tmpPos;
               return;
            }
         }
      }
   }

   inline bool searchData(int  *pDataInfoStackPtr,
                          int  &foundDataInfoStackPos)
   {
      int *pTmpPtr;
      int tmpPos;

      for (pTmpPtr = mpMixDataInfoStack, tmpPos=0;
           pTmpPtr < mpMixDataInfoStackTop;
           pTmpPtr += mMixDataInfoStackStep, tmpPos++)
      {
         int input=0;
         while (input<mMixDataInfoStackStep && 
                pDataInfoStackPtr[input] == pTmpPtr[input])
         {
            input++;
         }
         if (input == mMixDataInfoStackStep)
         {
            foundDataInfoStackPos = tmpPos;
            return true;
         }
      }

      return false;
   }

   inline void moveDataUp(int startDataInfoStackPos, int dataInfoStackPos)
   {
      if (mpMixDataInfoProcessedStack[dataInfoStackPos] >= 0 &&
          dataInfoStackPos >= startDataInfoStackPos)
      {
         // If this output was already processed - swap commands.
         moveTopMixActionMix(dataInfoStackPos);
      }
   }

   MpAudioBuf::SpeechType mixSpeechTypes(MpAudioBuf::SpeechType src1,
                                         MpAudioBuf::SpeechType src2)
   {
      // If one of speech types is unknown, result is unknown.
      if (src1 == MpAudioBuf::MP_SPEECH_UNKNOWN ||
          src2 == MpAudioBuf::MP_SPEECH_UNKNOWN)
      {
         return MpAudioBuf::MP_SPEECH_UNKNOWN;
      }

      // If one of sources is tone, result is tone (though I'm not sure this
      // is fully correct)
      if (src1 == MpAudioBuf::MP_SPEECH_TONE ||
          src2 == MpAudioBuf::MP_SPEECH_TONE)
      {
         return MpAudioBuf::MP_SPEECH_TONE;
      }

      // Here we ignore MP_SPEECH_MUTED type, because it should not be mixed.
      if (src1 == MpAudioBuf::MP_SPEECH_MUTED ||
          src2 == MpAudioBuf::MP_SPEECH_MUTED)
      {
         assert(!"Muted audio should not be mixed!");
         return MpAudioBuf::MP_SPEECH_UNKNOWN;
      }

      // If active speech is mixed with active, silent or comfort noise
      // frame, result is active speech.
      if (src1 == MpAudioBuf::MP_SPEECH_ACTIVE ||
          src2 == MpAudioBuf::MP_SPEECH_ACTIVE)
      {
         return MpAudioBuf::MP_SPEECH_ACTIVE;
      }

      // If silent is mixed with silent of comfort noise, result is silent.
      if (src1 == MpAudioBuf::MP_SPEECH_SILENT ||
          src2 == MpAudioBuf::MP_SPEECH_SILENT)
      {
         return MpAudioBuf::MP_SPEECH_SILENT;
      }

      // At this point only remaining case is that both frames are comfort noise.
      if (src1 != MpAudioBuf::MP_SPEECH_COMFORT_NOISE ||
          src2 != MpAudioBuf::MP_SPEECH_COMFORT_NOISE)
      {
         assert(!"Unknown frame speech type when mixing!");
         return MpAudioBuf::MP_SPEECH_UNKNOWN;
      }

      return MpAudioBuf::MP_SPEECH_COMFORT_NOISE;
   }

//@}

   MpBridgeAccum* mpMixAccumulator;    ///< Accumulator to store sum of all inputs.
                    ///< have size of mSamplesPerFrame. Used in doMix() only.

   UtlBoolean mMixSilence; ///< Should Bridge ignore or mix frames marked as silence?

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
