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

#ifndef _MprVad_h_
#define _MprVad_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpVadBase;

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

     /// Change VAD algorithm to the given one.
   OsStatus chageVadAlgorithm(const UtlString& namedResource, 
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
      MPRM_CHANGE_VAD = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START
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

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprVad_h_
