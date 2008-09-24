//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprRtpStartSendMsg_h_
#define _MprRtpStartSendMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MprRtpStartSendMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprRtpStartSendMsg(const UtlString& targetResourceName,
                      SdpCodec* audioCodec,
                      SdpCodec* dtmfCodec,
                      OsSocket& rRtpSocket,
                      OsSocket& rRtcpSocket)
      : MpResourceMsg(MPRM_START_SEND_RTP, targetResourceName)
      , mAudioCodecSet(FALSE)
      , mDtmfCodecSet(FALSE)
      , mAudioCodec()
      , mDtmfCodec()
      , mpRtpSocket(&rRtpSocket)
      , mpRtcpSocket(&rRtcpSocket)
   {

       if(audioCodec)
       {
           mAudioCodec = *audioCodec;
           mAudioCodecSet = TRUE;
       }
       if(dtmfCodec)
       {
           mDtmfCodecSet = TRUE;
           mDtmfCodec = *dtmfCodec;
       }
   };

   /// Copy constructor
   MprRtpStartSendMsg(const MprRtpStartSendMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mAudioCodecSet(resourceMsg.mAudioCodecSet)
      , mDtmfCodecSet(resourceMsg.mDtmfCodecSet)
      , mAudioCodec(resourceMsg.mAudioCodec)
      , mDtmfCodec(resourceMsg.mDtmfCodec)
      , mpRtpSocket(resourceMsg.mpRtpSocket)
      , mpRtcpSocket(resourceMsg.mpRtcpSocket)
   {
   };

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprRtpStartSendMsg(*this); 
   }

   /// Destructor
   ~MprRtpStartSendMsg() 
   {
   };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprRtpStartSendMsg& operator=(const MprRtpStartSendMsg& rhs)
   {
        if(&rhs == this)
        {
            return(*this);
        }

        mAudioCodecSet = rhs.mAudioCodecSet;
        mDtmfCodecSet = rhs.mDtmfCodecSet;
        mAudioCodec = rhs.mAudioCodec;
        mDtmfCodec = rhs.mDtmfCodec;
        mpRtpSocket = rhs.mpRtpSocket;
        mpRtcpSocket = rhs.mpRtcpSocket;

        return *this;
   }

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   void getCodecs(SdpCodec*& audioCodec, SdpCodec*& dtmfCodec)
   {
       if(mAudioCodecSet)
       {
           audioCodec = &mAudioCodec;
       }
       else
       {
           audioCodec = NULL;
       }

       if(mDtmfCodecSet)
       {
           dtmfCodec = &mDtmfCodec;
       }
       else
       {
           dtmfCodec = NULL;
       }
   }

   OsSocket* getRtpSocket(){return(mpRtpSocket);};

   OsSocket* getRtcpSocket(){return(mpRtcpSocket);};

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlBoolean mAudioCodecSet;
    UtlBoolean mDtmfCodecSet;
    SdpCodec mAudioCodec;
    SdpCodec mDtmfCodec;
    OsSocket* mpRtpSocket;
    OsSocket* mpRtcpSocket;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpStartSendMsg_h_
