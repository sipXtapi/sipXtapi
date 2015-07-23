//  
// Copyright (C) 2008-2015 SIPez LLC.  All rights reserved.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


#ifndef _MprNotchFilter_h_
#define _MprNotchFilter_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpResourceMsg.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class MprNotchFilter : public MpAudioResource
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:
/* =============================== CREATORS =============================== */
///@name Creators
//@{
     /// Constructor
   MprNotchFilter(const UtlString& rName);

     /// Destructor
   ~MprNotchFilter();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

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

     /// @copydoc MpResource::handleMessage
   UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// @copydoc MpAudioResource::doProcessFrame
   UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                             MpBufPtr outBufs[],
                             int inBufsSize,
                             int outBufsSize,
                             UtlBoolean isEnabled,
                             int samplesPerFrame,
                             int samplesPerSecond);


/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   static const float DEFAULT_TARGET_FREQ;
   static const float DEFAULT_SAMPLING_RATE;
   static const float DEFAULT_BW;

   float mTargetFreq;
   float mSampleRate;
   float mBW;
   float mA0;
   float mA1;
   float mA2;
   float mB1;
   float mB2;
   float mZ1;
   float mZ2;

   MpFlowGraphBase* mpFlowGraph;
   void initFilter();
   void doFiltering(const MpAudioSample *x, MpAudioSample *y, int n);
   UtlBoolean filterSamples(MpBufPtr inBufs[], int inBufsSize,
                            MpBufPtr outBufs[], int outBufsSize,
                            int samplesPerFrame);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprNotchFilter_h_
