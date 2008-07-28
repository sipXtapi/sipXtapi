// Copyright 2008 AOL LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA. 
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
#include "mediaInterface/IMediaTransportAdapter.h"

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
                      IMediaTransportAdapter* pAdapter)
      : MpResourceMsg(MPRM_START_SEND_RTP, targetResourceName)
      , mAudioCodecSet(FALSE)
      , mDtmfCodecSet(FALSE)
      , mAudioCodec()
      , mDtmfCodec()
      , mpSocketAdapter(pAdapter)
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
      , mpSocketAdapter(resourceMsg.mpSocketAdapter)
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
        mpSocketAdapter = rhs.mpSocketAdapter;

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

   IMediaTransportAdapter* getSocketAdapter() { return mpSocketAdapter; }


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
    IMediaTransportAdapter* mpSocketAdapter;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpStartSendMsg_h_
