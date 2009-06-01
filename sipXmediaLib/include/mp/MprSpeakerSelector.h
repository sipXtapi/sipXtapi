//
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT sipez DOT com>

#ifndef _MprSpeakerSelector_h_
#define _MprSpeakerSelector_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResource.h>
#include <mp/MpResourceMsg.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class MpSpeakerSelectBase;

/**
*  @brief The Speaker Selection resource.
*
*/
class MprSpeakerSelector : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprSpeakerSelector(const UtlString& rName,
                      int maxInputs,
                      int maxOutputs,
                      int maxActiveSpeakers,
                      const UtlString &algorithmName = "");

     /// Constructor
   MprSpeakerSelector(const UtlString& rName,
                      int maxInputs,
                      int maxOutputs,
                      int maxActiveSpeakers,
                      MpSpeakerSelectBase *pSS);
     /**<
     *  Use this constructor for debug purposes only! Usage of constructor
     *  specifying Speaker Selection algorithm by name is strongly encouraged.
     */

     /// Destructor
   virtual ~MprSpeakerSelector();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Change Speaker Selection algorithm to the given one.
   OsStatus chageAlgorithm(const UtlString& namedResource, 
                           OsMsgQ& fgQ,
                           const UtlString &algorithmName);
     /**<
     *  @param[in] algorithmName - name of the Speaker Selection algorithm
     *             to use. If empty string of incorrect name is given, default
     *             Speaker Selection algorithm will be used and no error is
     *             generated.
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
      MPRM_CHANGE_ALGORITHM = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
   } AddlMsgTypes;
   
   struct RankIndexPair
   {
      unsigned mRank;  ///< Rank of a speaker, as set by Speaker Selection algorithm.
      int      mIndex; ///< Index of a speaker in mpFrameParams array.
   };

   UtlBoolean           mDynamicInputMapping; ///< Are inputs to outputs mapped dynamically
                                     ///< or statically one-to-one?
   int                  mNumStreams; ///< Maximum number of speakers we can process.
   MpSpeakerSelectBase *mpSS;        ///< Instance of Speaker Selection algorithm.
   UtlBoolean           mSsFresh;    ///< Have algorithm been just initialized or reseted?
   MpSpeechParams     **mpFrameParams; ///< Array of pointers to frame parameters
                                     ///< for incoming frames.
   int                  mMaxActiveSpeakers; ///< Number of speakers we want to mix.
   RankIndexPair       *mTopRanks;   ///< Array of top ranked speakers, sorted by rank.
   int                 *mInToOutMap; ///< Mapping of inputs to outputs.
   int                 *mOutToInMap; ///< Mapping of outputs to inputs.
   uint32_t             mMapTime;
   int                 *mChangeMapTime;

     /// @copydoc MpResource::connectInput()
   UtlBoolean connectInput(MpResource& rFrom, int fromPortIdx, int toPortIdx);

     /// @copydoc MpResource::connectOutput()
   UtlBoolean connectOutput(MpResource& rTo, int toPortIdx, int fromPortIdx);

     /// @copydoc MpResource::disconnectInput()
   UtlBoolean disconnectInput(int outPortIdx);

     /// @copydoc MpResource::disconnectOutput()
   UtlBoolean disconnectOutput(int outPortIdx);

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

     /// @copydoc MpAudioResource::doProcessFrame()
   UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                             MpBufPtr outBufs[],
                             int inBufsSize,
                             int outBufsSize,
                             UtlBoolean isEnabled,
                             int samplesPerFrame,
                             int samplesPerSecond);

     /// @copydoc MpResource::handleMessage()
   UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// @copydoc MpResource::handleEnable()
   UtlBoolean handleEnable();

     /// @copydoc MpResource::handleDisable()
   UtlBoolean handleDisable();

     /// Handle MPRM_CHANGE_ALGORITHM message.
   UtlBoolean handleChangeAlgorithm(const UtlString &algorithmName);

     /// Enable participants in algorithm whose streams are connected.
   void enableConnectedStreams();
     /**<
     *  This method is usually called when algorithm is reseted or changed
     *  when resource is already connected.
     */

     /// Peek top ranked speakers from the list.
   static void peekTopSpeakers(MpSpeechParams **frameParams, int frameParamsNum,
                               RankIndexPair *topRanks, int topRanksNum);

   void updateMapping(RankIndexPair *topRanks, int topRanksNum);
   inline int getOldestOutput();
   inline int getOutputForInput(int inputNum);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprSpeakerSelector(const MprSpeakerSelector& rMprSpeakerSelector);

     /// Assignment operator (not implemented for this class)
   MprSpeakerSelector& operator=(const MprSpeakerSelector& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeakerSelector_h_
