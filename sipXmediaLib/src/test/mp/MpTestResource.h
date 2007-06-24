//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpTestResource_h_
#define _MpTestResource_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class mpFlowGraphMsg;

/**
 * @brief Descendant of the MpAudioResource class used for testing.
 *
 * @nosubgrouping
 */
class MpTestResource : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /// Snapshot of the args of the last call to the doProcessFrame().
   /**
    * Structure holding a snapshot of the args passed to the most recent 
    * call to doProcessFrame().
    */
   struct DoProcessArgs
   {
      MpBufPtr* inBufs;
      MpBufPtr* outBufs;
      int       inBufsSize;
      int       outBufsSize;
      UtlBoolean isEnabled;
      int       samplesPerFrame;
      int       samplesPerSecond;
   };

   int            mGenOutBufMask;
   int            mProcessInBufMask;
   MpFlowGraphMsg mLastMsg;
   DoProcessArgs  mLastDoProcessArgs;

   enum MpTestOutSignal
   {
      MP_TEST_SIGNAL_NULL,
      MP_TEST_SIGNAL_SQUARE
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MpTestResource(const UtlString& rName
                 , int minInputs, int maxInputs
                 , int minOutputs, int maxOutputs
                 , int samplesPerFrame=80, int samplesPerSec=8000);

     /// Destructor
   virtual
   ~MpTestResource();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
     /// Sends a test message to this resource.
   void sendTestMessage(void* ptr1, void* ptr2, int int3, int int4);

     /// Specify the genOutBufMask.
   void setGenOutBufMask(int mask);
     /**<
      * For each bit in the genOutBufMask that is set, if there is a
      * resource connected to the corresponding output port, doProcessFrame()
      * will create an output buffer on that output port.
      */

     /// Specify the processInBufMask.
   void setProcessInBufMask(int mask);
     /**<
      * For each bit in the processInBufMask that is set, doProcessFrame()
      * will pass the input buffer from the corresponding input port,
      * straight through to the corresponding output port.  If nothing is
      * connected on the corresponding output port, the input buffer will
      * be deleted.
      */

     /// Set type of signal, generated on outputs.
   void setOutSignalType(MpTestOutSignal signalType);

     /// Set period of signal (for signal types, supporting period).
   void setSignalPeriod(int outputIndex, int periodInSamples);

     /// Set amplitude of signal (for signal types, supporting amplitude).
   void setSignalAmplitude(int outputIndex, int maxMinValue);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     /// Returns the count of the number of frames processed by this resource.
   int numFramesProcessed(void);

     /// @brief Returns the count of the number of messages successfully
     /// processed by this resource.
   int numMsgsProcessed(void);

   MpAudioSample getSquareSampleValue(int outputIndex,
                                      int sampleIndex);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* ============================ UTILITY =================================== */
///@name Utility
//@{

   static
   MpAudioSample squareSampleValue(int squareWavePeriod,
                                   int squareWaveAmplitude,
                                   int sampleIndex);

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   int            mProcessedCnt;      ///< Number of processed frames
   int            mMsgCnt;            ///< Number received messages
   MpTestOutSignal mSignalType;       ///< Output signal type
   int*           mpSignalPeriod;     ///< Period of signal if supported (in samples)
   int*           mpSignalAmplitude;  ///< Magnitude of signal if supported

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Processes the next frame interval's worth of media.
   UtlBoolean doProcessFrame(MpBufPtr inBufs[], MpBufPtr outBufs[],
                            int inBufsSize, int outBufsSize,
                            UtlBoolean isEnabled, int samplesPerFrame=80,
                            int samplesPerSecond=8000);

     /// Handles messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MpTestResource(const MpTestResource& rMpTestResource);

     /// Assignment operator (not implemented for this class)
   MpTestResource& operator=(const MpTestResource& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpTestResource_h_
