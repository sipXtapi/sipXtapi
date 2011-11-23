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

#ifndef _MprRtpDispatcherActiveSsrcs_h_
#define _MprRtpDispatcherActiveSsrcs_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MprRtpDispatcher.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlSList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MprDejitter;

/**
*  @brief Class for RTP stream abstraction.
*
*  @nosubgrouping
*/
class MprRtpDispatcherActiveSsrcs : public MprRtpDispatcher
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprRtpDispatcherActiveSsrcs(const UtlString& name, int connectionId,
                               int streamsNum);
     /**<
     *  @param name - name of this RTP dispatcher.
     *  @param connectionId - connection ID to be used in notifications.
     *  @param pMsgQ - notification dispatcher to send notifications to.
     *  @param streamsNum - maximum number of the RTP streams this dispatcher
     *                      will be able to handle.
     */

     /// Destructor
   virtual
   ~MprRtpDispatcherActiveSsrcs();

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

   int             mRtpStreamsNum;   ///< Number of RTP streams we can handle.
   MpRtpStream    *mpStreamsArray;   ///< Array of all RTP streams.
   UtlHashBag      mActiveStreams;   ///< Hash bag of active RTP streams.
   UtlSList        mInactiveStreams; ///< List of inactive RTP streams.
                     ///< RTP stream should be contained in active XOR inactive
                     ///< list. Stream MUST not be listed in active and inactive
                     ///< lists at one time.

   int             mNumPushed;       ///< Total RTP packets received
   int             mNumDropped;      ///< RTP packets dropped due to SSRC mismatch

     /// Find stream with given SSRC.
   MpRtpStream *lookupRtpStream(unsigned int ssrc, const in_addr &fromIp,
                                int fromPort);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Copy constructor (not implemented for this class)
   MprRtpDispatcherActiveSsrcs(const MprRtpDispatcherActiveSsrcs& rMprRtpDispatcherActiveSsrcs);

     /// Assignment operator (not implemented for this class)
   MprRtpDispatcherActiveSsrcs& operator=(const MprRtpDispatcherActiveSsrcs& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpDispatcherActiveSsrcs_h_
