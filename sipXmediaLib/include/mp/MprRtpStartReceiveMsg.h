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

#ifndef _MprRtpStartReceiveMsg_h_
#define _MprRtpStartReceiveMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpResourceMsg.h"
#include "sdp/SdpCodec.h"
#include "os/OsMsg.h"
#include "os/OsNotification.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MprRtpStartReceiveMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprRtpStartReceiveMsg(const UtlString& targetResourceName,
                         SdpCodec* codecs[],
                         int numCodecs,
                         OsSocket& rRtpSocket,
                         OsSocket& rRtcpSocket,
                         OsNotification* pNotifier = NULL)
      : MpResourceMsg(MPRM_START_RECEIVE_RTP, targetResourceName)
      , mpCodecs(NULL)
      , mNumCodecs(0)
      , mpRtpSocket(&rRtpSocket)
      , mpRtcpSocket(&rRtcpSocket)
      , mpNotifier(pNotifier)
   {
       assert(numCodecs > 0);
       if(numCodecs > 0)
       {
           mpCodecs = new SdpCodec*[numCodecs];
           int codecIndex;
           for(codecIndex = 0; codecIndex < numCodecs; codecIndex++)
           {
               if(codecs[codecIndex])
               {
                   mpCodecs[codecIndex] = 
                       new SdpCodec(*(codecs[codecIndex]));
               }

               // This should never occur:
               else
               {
                   assert(codecs[codecIndex] != NULL);
                   mpCodecs[codecIndex] = NULL;
               }
           }
           mNumCodecs = numCodecs;
       }
   };

   /// Copy constructor
   MprRtpStartReceiveMsg(const MprRtpStartReceiveMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mpCodecs(NULL)
      , mpRtpSocket(resourceMsg.mpRtpSocket)
      , mpRtcpSocket(resourceMsg.mpRtcpSocket)
      , mpNotifier(resourceMsg.mpNotifier)
   {
       assert(resourceMsg.mNumCodecs > 0);
       if(resourceMsg.mNumCodecs > 0)
       {
           // Make a codec array to copy into this message
           mpCodecs = new SdpCodec*[resourceMsg.mNumCodecs];
           int codecIndex;
           for(codecIndex = 0; codecIndex < resourceMsg.mNumCodecs; codecIndex++)
           {
               if(resourceMsg.mpCodecs[codecIndex])
               {
                   mpCodecs[codecIndex] = new SdpCodec(*(resourceMsg.mpCodecs[codecIndex]));
               }

               // This should never occur:
               else
               {
                   assert(resourceMsg.mpCodecs[codecIndex] != NULL);
                   mpCodecs[codecIndex] = NULL;
               }
           }
           mNumCodecs = resourceMsg.mNumCodecs;
       }
   };

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprRtpStartReceiveMsg(*this); 
   }

   /// Destructor
   ~MprRtpStartReceiveMsg() 
   {
       if(mpCodecs)
       {
           int codecIndex;
           for(codecIndex = 0; codecIndex < mNumCodecs; codecIndex++)
           {
               if(mpCodecs[codecIndex])
               {
                   delete mpCodecs[codecIndex];
                   mpCodecs[codecIndex] = NULL;
               }
           }
           delete[] mpCodecs;
           mpCodecs = NULL;
       }
   };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprRtpStartReceiveMsg& operator=(const MprRtpStartReceiveMsg& rhs)
   {
      if (this == &rhs) 
         return *this;  // handle the assignment to self case

      MpResourceMsg::operator=(rhs);  // assign fields for parent class

      // The target array is bigger free up the spare SdpCodecs
      int codecIndex;
      for(codecIndex = rhs.mNumCodecs; codecIndex < mNumCodecs; codecIndex++)
      {
          delete mpCodecs[codecIndex];
          mpCodecs[codecIndex] = NULL;
      }

      // The target array is not big enough, allocate a bigger one
      if(mNumCodecs < rhs.mNumCodecs && rhs.mNumCodecs > 0)
      {
          SdpCodec** tempCodecArray = mpCodecs;
          mpCodecs = new SdpCodec*[rhs.mNumCodecs];
          // Move the existing codecs to the bigger array to avoid
          // allocation of new ones
          for(codecIndex = 0; codecIndex < mNumCodecs; codecIndex++)
          {
              mpCodecs[codecIndex] = tempCodecArray[codecIndex];
              tempCodecArray[codecIndex] = NULL;
          }
          delete[] tempCodecArray;
          tempCodecArray = NULL;
      }

      // Copy the codecs from the source to the target
      for(codecIndex = 0; codecIndex < rhs.mNumCodecs; codecIndex++)
      {
          if(mpCodecs[codecIndex])
          {
              *(mpCodecs[codecIndex]) = *(rhs.mpCodecs[codecIndex]);
          }
          else
          {
              mpCodecs[codecIndex] = new SdpCodec(*(rhs.mpCodecs[codecIndex]));
          }
      }

      mNumCodecs = rhs.mNumCodecs;
      mpRtpSocket = rhs.mpRtpSocket;
      mpRtcpSocket = rhs.mpRtcpSocket;

      return *this;
   }

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   void getCodecArray(int& codecCount, SdpCodec**& codecs)
   {
       codecCount = mNumCodecs;
       codecs = mpCodecs;
   }

   OsSocket* getRtpSocket(){return(mpRtpSocket);};

   OsSocket* getRtcpSocket(){return(mpRtcpSocket);};
   OsNotification* getNotifier(){return(mpNotifier);};

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SdpCodec** mpCodecs;
    int mNumCodecs;
    OsSocket* mpRtpSocket;
    OsSocket* mpRtcpSocket;
    OsNotification* mpNotifier;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpStartReceiveMsg_h_
