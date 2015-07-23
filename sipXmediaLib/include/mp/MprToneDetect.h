//  
// Copyright (C) 2006-2015 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprToneDetect_h_
#define _MprToneDetect_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <mp/MpFlowGraphMsg.h>
#include <mp/MpResourceMsg.h>
#include <mp/MpAudioResource.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "Tone Detector" media processing resource
*
*/
class MprToneDetect : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprToneDetect(const UtlString& rName);

     /// Destructor
   virtual ~MprToneDetect();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   static OsStatus changeTargetFrequency(const UtlString& namedResource, 
                                         OsMsgQ& fgQ,
                                         int targetFreq);

   static OsStatus changeDetectionThreshold(const UtlString& namedResource, 
                                            OsMsgQ& fgQ,
                                            int threshold);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
   // ideally this should do a lookup on the named resource and
   // return the value for that
   double getAvgMagnitude(const UtlString& resName);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum
   {
      MPRM_CHANGE_TARGET_FREQ = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_CHANGE_THRESHOLD
   };

   typedef enum
   {
      SET_TARGET_FREQ = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_THRESHOLD
   } AddlMsgTypes;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond);

     /// Handle flowgraph messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& fgMsg);

     /// Handle resource messages for this resource.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// @brief Associates this resource with the indicated flow graph.
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);
     /**<
     *  We use this overloaded method for initialization of some of our member
     *  variables, which depend on flowgraph's properties (like frame size).
     *
     *  @retval OS_SUCCESS - for now, this method always returns success
     */

     /// Copy constructor (not implemented for this class)
   MprToneDetect(const MprToneDetect& rMprToneDetect);

     /// Assignment operator (not implemented for this class)
   MprToneDetect& operator=(const MprToneDetect& rhs);

   double getAvgMag() const;

   static const double DEFAULT_TARGET_FREQ;
   static const double DEFAULT_SAMPLING_RATE;
   static const double DEFAULT_THRESHOLD;
   static const unsigned DEFAULT_BLOCK_SIZE;
   static const int DEFAULT_WINDOW_SIZE;

   double mCoeff;
   double mQ1;
   double mQ2;
   double mThreshold;
   double mTargetFreq;
   unsigned mBlockCnt;
   bool mCurTd;
   bool mToneSignaled;
   double mAvg3Mag;
   double mSampleRate;
   unsigned mBlockSize;
   int mWindowSize;
   MpFlowGraphBase* mpFlowGraph;

   double *mFilterData;
   void resetGoertzel(void);
   void initGoertzel(void);
   void processSample(MpAudioSample sample);
   double getMagnitudeSquared(void);
   void initFilter();
   double filterSample(MpAudioSample sample, uint32_t n);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToneDetect_h_
