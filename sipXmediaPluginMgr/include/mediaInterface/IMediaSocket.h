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
// Copyright (C) 2005-2008 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IMediaSocket_h_
#define _IMediaSocket_h_

#include "os/IOsNatSocket.h"

// DEFINES
#define TYPE_AUDIO_RTP    0
#define TYPE_AUDIO_RTCP   1
#define TYPE_VIDEO_RTP    2
#define TYPE_VIDEO_RTCP   3

#define TRANSPORT_DUMP
#undef TRANSPORT_DUMP
#ifdef TRANSPORT_DUMP
#define TRANSPORT_DUMP_FILE  "C:\\transport-debug.txt" 
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef struct RTP_HEADER
{
    unsigned char vpxcc ;
    unsigned char mp ;

    unsigned short seq_number ;
    unsigned long  timestamp ;
    unsigned long  SSRC ;
} RTP_HEADER ;

typedef struct RTP_MONITOR_CONEXT
{
    bool           bInitialized ;
    unsigned char  payloadType ;
    unsigned short seq_number ;
    unsigned long  SSRC ;

    UtlString      fromAddress ;
    int            fromPort ;
    unsigned int   mismatches ;
} RTP_MONITOR_CONEXT ;

/**
 * Generic interface representing a media transport object.
 * Implemented by VoiceEngineDatagramSocket, VoiceEngineConnectionSocket,
 * and any other class which provides RTP transport of media.
 */
class IMediaSocket
{
protected:
    int       mAudioChannel ;
    int       mVideoChannel ;
    UtlString mPreferredReceiveAddress  ;
    int       mPreferredReceivePort ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /**
     * Default Constructor
     */
    IMediaSocket()
    {
        mAudioChannel = -1 ;
        mVideoChannel = -1 ;
        mPreferredReceivePort = 0 ;
    }

    /**
     * virtual destructor.
     */
    virtual ~IMediaSocket() {};

/* ============================== ACCESSORS =============================== */

    void setAudioChannel(int channelId)
        { mAudioChannel = channelId ; } ;

    int getAudioChannel() const
        { return mAudioChannel ; } ;

    void setVideoChannel(int channelId)
        { mVideoChannel = channelId ; } ;

    int getVideoChannel() const
        { return mVideoChannel ; } ;    

    int getChannel() const
        { return ((mAudioChannel != -1) ? mAudioChannel : mVideoChannel) ; } ;
        
    void setPreferredReceiveAddress(const char*szAddress, int port) 
        {
            mPreferredReceiveAddress = szAddress ;
            mPreferredReceivePort = port ;
        } ;

    void getPreferredReceiveAddress(UtlString& address, int& port) 
        {
            address = mPreferredReceiveAddress ;
            port = mPreferredReceivePort  ;
        } ;

    virtual OsSocket* getSocket() = 0 ;

/* ============================ MANIPULATORS ============================== */
    
    /**
     * Reads an RTP packet and 'pushes' it to media processing.
     */
    virtual bool pushPacket() = 0;

    /**
     * Enable media transport reading & writing.
     */
    virtual void setEnabled(bool bEnabled) = 0;
 
};


inline IMediaSocket* getMediaSocketPtr(IOsNatSocket* pStunSocket)
{
    return dynamic_cast<IMediaSocket*>(pStunSocket->getSocket());
}

#endif // #ifndef _IMediaSocket_h_

