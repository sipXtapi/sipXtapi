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

#ifndef _MprVoiceActivityNotifier_h_
#define _MprVoiceActivityNotifier_h_

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
*  @brief Voice Activity notifier resource.
*
*  @nosubgrouping
*/
class MprVoiceActivityNotifier : public MpAudioResource
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

     /// Default period (in ms) to send energy level notifications.
   static const int smDefaultLevelPeriodMs;
     /// Default duration of silence (in ms) before STOPPED notification is sent.
   static const int smDefaultInactivityTimeoutMs;

/* =============================== CREATORS =============================== */
///@name Creators
//@{

     /// Constructor
   MprVoiceActivityNotifier(const UtlString& rName,
                            int notificationPeriodMs = smDefaultLevelPeriodMs,
                            int inactivityTimeoutMs = smDefaultInactivityTimeoutMs);
     /**<
     *  @param[in] notificationPeriodMs - see chageVadAlgorithm() for description.
     /// Default 
     static const int smDefaultLevelPeriodMs;
     /// Default length of silence (in ms) before STOPPED notification is sent.
     */

     /// Destructor
   ~MprVoiceActivityNotifier();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

     /// Change period between energy level notifications.
   static
   OsStatus chageNotificationPeriod(const UtlString& namedResource, 
                                    OsMsgQ& fgQ,
                                    int notificationPeriodMs);

     /// Change duration of silence (in ms) before STOPPED notification is sent.
   static
   OsStatus changeInactivityTimeout(const UtlString& namedResource, 
                                    OsMsgQ& fgQ,
                                    int inactivityTimeoutMs);

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
      MPRM_CHANGE_NOTIFICATION_PERIOD = MpResourceMsg::MPRM_EXTERNAL_MESSAGE_START,
      MPRM_CHANGE_INACTIVITY_TIMEOUT
   };

   enum StreamState
   {
      STREAM_ACTIVE,
      STREAM_SILENT
   };

   int mLevelPeriodMs;          ///< Number of milliseconds between level notifications.
   int mLevelPeriodSmp;         ///< Number of samples between level notifications.
   int mLevelSamplesPassed;     ///< Number of samples since last level notification.
   int mInactivityTimeoutMs;    ///< Number of milliseconds of silence before STOP notification.
   int mInactivityTimeoutSmp;   ///< Number of samples of silence before STOP notification.
   int mInactivitySamples;      ///< Number of samples since last active packet.
   int mEnergyLevelMax;         ///< Maximum of the energy level since the last notification.
   StreamState mStreamState;    ///< Stream activity state.

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

     /// Handle MPRM_CHANGE_NOTIFICATION_PERIOD message.
   UtlBoolean handleChageNotificationPeriod(int periodMs);

     /// Handle MPRM_CHANGE_INACTIVITY_TIMEOUT message.
   UtlBoolean handleChangeInactivityTimeout(int timeout);

     /// @copydoc MpResource::setFlowGraph()
   OsStatus setFlowGraph(MpFlowGraphBase* pFlowGraph);

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprVoiceActivityNotifier_h_
