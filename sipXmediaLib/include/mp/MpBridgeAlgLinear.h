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

#ifndef _MpBridgeAlgLinear_h_
#define _MpBridgeAlgLinear_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpBridgeAlgBase.h"

// DEFINES
#define TEST_PRINT_MIXING
#undef  TEST_PRINT_MIXING

#define TEST_PRINT_MIXING_BINARY
#undef  TEST_PRINT_MIXING_BINARY

#define TEST_PRINT_MIXING_CONTRIBUTORS
#undef  TEST_PRINT_MIXING_CONTRIBUTORS

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
class MpBridgeAlgLinear : public MpBridgeAlgBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor.
   MpBridgeAlgLinear(int inputs, int outputs, UtlBoolean mixSilence,
                     int samplesPerFrame);

     /// Destructor.
   ~MpBridgeAlgLinear();

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
   MpSpeechType*  mpMixDataSpeechType; ///< Speech type of data frames in mpMixDataStack
   MpBridgeAccum* mpMixDataAmplitude; ///< Amplitude of data frames in mpMixDataStack
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

//@}


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpBridgeAlgLinear_h_
