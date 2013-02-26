//  
// Copyright (C) 2007-2011 SIPez LLC.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

#ifndef _MprRtpDispatcherIpAffinity_h_
#define _MprRtpDispatcherIpAffinity_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MprRtpDispatcher.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpRtpStream;
class MprDejitter;

/**
*  @brief Class for RTP stream dispatcher abstraction.
*/
class MprRtpDispatcherIpAffinity : public MprRtpDispatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprRtpDispatcherIpAffinity(const UtlString& rName, int connectionId);
     /**<
     *  @param name - name of this RTP dispatcher.
     *  @param connectionId - connection ID to be used in notifications.
     *  @param pMsgQ - notification dispatcher to send notifications to.
     */

     /// Destructor
   virtual
   ~MprRtpDispatcherIpAffinity();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// @copydoc MprRtpDispatcher::pushPacket()
   OsStatus pushPacket(MpRtpBufPtr &pRtp);

     /// @copydoc MprRtpDispatcher::checkRtpStreams()
   void checkRtpStreamsActivity();

     /// @copydoc MprRtpDispatcher::connectOutput()
   UtlBoolean connectOutput(int outputIdx, MpResource* pushRtpToResource);

     /// @copydoc MprRtpDispatcher::disconnectOutput()
   UtlBoolean disconnectOutput(int outputIdx);

     /// Inform this object of its sibling ToNet's destination.
   void setPreferredIp(unsigned long address, int port);

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

   MpRtpStream     mRtpStream;     ///< Our RTP stream instance (single for this
                                   ///< dispatcher)
   MprDejitter    *mpDejitter;     ///< Dejitter for our single decoder.

   UtlBoolean      mPrefSsrcValid; ///< Is mPrefSsrc valid?
   unsigned long   mRtpDestIp;     ///< Where this connection is sending TO
   int             mRtpDestPort;   ///< Port   "      "

   int             mNumPushed;              ///< Total RTP pkts received
   int             mNumDropped;             ///< RTP pkts dropped due to SSRC mismatch
   int             mTotalWarnings;          ///< Total RTP pkts from non-pref'd
   int             mNumWarnings;            ///< Current consecutive "   "
   int             mNumNonPrefPackets;      ///< Consecutive pkts from non-pref'd
   int             mRtpDestMatchIpOnlySsrc; ///< Last SSRC from same IP as pref'd
   UtlBoolean      mRtpDestMatchIpOnlySsrcValid;
   int             mRtpOtherSsrc;           ///< Last SSRC from diff IP as pref'd
   UtlBoolean      mRtpOtherSsrcValid;

   static const int SSRC_SWITCH_MISMATCH_COUNT;

     /// Set SSRC we want to receive.
   int setPrefSsrc(unsigned int newSsrc);
     /// Get SSRC we want to receive.
   RtpSRC getPrefSsrc(void);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprRtpDispatcherIpAffinity(const MprRtpDispatcherIpAffinity& rMprRtpDispatcherIpAffinity);

     /// Assignment operator (not implemented for this class)
   MprRtpDispatcherIpAffinity& operator=(const MprRtpDispatcherIpAffinity& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpDispatcherIpAffinity_h_
