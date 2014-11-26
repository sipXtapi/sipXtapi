//  
// Copyright (C) 2008-2014 SIPez LLC. All right reserved.
//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license.
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprVad_h_
#define _MprVad_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpAudioResource.h>
#include <mp/MpResourceMsg.h>
#include <mp/MprDecode.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpVadBase;
class UtlSerialized;

/**
*  @brief Voice Activity Detection resource.
*
*  @nosubgrouping
*/
class MprVad : public MpAudioResource
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   MprVad(const UtlString& rName, const UtlString &vadAlgorithm = "");
     /**<
     *  @param[in] vadAlgorithm - see chageVadAlgorithm() for description.
     */

     /// Destructor
   ~MprVad();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Set VAD parameter
   static OsStatus setVadParameter(const UtlString& parameterName,
                                   int parameterValue,
                                   const UtlString& resourceName,
                                   OsMsgQ& flowgraphQueue);
   /**
     *  Sets the named parameter on the named resource containing a
     *  VAD evaluator (e.g. MprVad or MprDecode).
     *  @param[in] parameterName - name of the VAD parameter to set
     *  @param[in] parameterValue - new value to assign to named parameter
     *  @param[in] resourceName - the name of the resource which contains 
     *             the VAD whose parameter is to be set.
     *  @param[in] flowgraphQueue - the queue of the flowgraph containing 
     *             the named resource.
     */

     /// Change VAD algorithm to the given one.
   static OsStatus changeVadAlgorithm(const UtlString& namedResource, 
                                     OsMsgQ& fgQ,
                                     const UtlString &vadAlgorithm);
     /**<
     *  @param[in] vadAlgorithm - name of the VAD algorithm to use. If empty
     *             string of incorrect name is given, default VAD algorithm
     *             will be used and no error is generated.
     */

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

   enum
   {
      MPRM_CHANGE_VAD = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      // WARNING: be sure to not conflict with MprDecode::MPRM_SET_VAD_PARAM

      MPRM_SET_VAD_PARAM = MprDecode::MPRM_SET_VAD_PARAM
   };

   MpVadBase *mpVad;   ///< Instance of the VAD algorithm to use.

     /// @copydoc MpAudioResource::doProcessFrame
   UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                             MpBufPtr outBufs[],
                             int inBufsSize,
                             int outBufsSize,
                             UtlBoolean isEnabled,
                             int samplesPerFrame,
                             int samplesPerSecond);

     /// @copydoc MpResource::handleMessage
   UtlBoolean handleMessage(MpResourceMsg& rMsg);

     /// Handle MPRM_CHANGE_VAD message.
   UtlBoolean handleChageVadAlgorithm(const UtlString &vadAlgorithm);

     /// Handle message to set VAD parameter
   UtlBoolean handleSetVadParam(UtlSerialized& serialData);

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprVad_h_
