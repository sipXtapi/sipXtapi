//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _IMediaSocket_h_
#define _IMediaSocket_h_


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
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ MANIPULATORS ============================== */

    /**
     * virtual destructor.
     */
    virtual ~IMediaSocket() {};
    
    /**
     * Reads an RTP packet and 'pushes' it to media processing.
     */
    virtual void pushPacket() = 0;
    
    /**
     * Reads an RTP packet and 'pushes' it to media processing.
     */
    virtual void pushPacket(char* rtpPacket, size_t packetSize, int port) = 0;

    /** 
     * Set the channel Id for video.
     *
     * @param channelId The VideoEngine channel id to be 
     *        associated with this transport object.
     */
    virtual void setVideoChannel(int channelId) = 0;
    
    /**
     * Timestamp the object with the last time that data
     * was read.
     */
    virtual void setLastTimeReadToNow() = 0;
    
    /**
     * Enable media transport reading & writing.
     */
    virtual void setEnabled(bool bEnabled) = 0;
    
    virtual void setPreferredReceiveAddress(const char*szAddress, int port) = 0;

};


inline IMediaSocket* getMediaSocketPtr(IStunSocket* pStunSocket)
{
    return dynamic_cast<IMediaSocket*>(pStunSocket->getSocket());
}

#endif // #ifndef _IMediaSocket_h_