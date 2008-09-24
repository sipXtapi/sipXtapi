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
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _IVoiceEngineMediaSocket_h_
#define _IVoiceEngineMediaSocket_h_

/**
 * Generic interface representing a media transport object.
 * Implemented by VoiceEngineDatagramSocket, VoiceEngineConnectionSocket,
 * and any other class which provides RTP transport for VoiceEngine.
 */
class IVoiceEngineMediaSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ MANIPULATORS ============================== */

    /**
     * Reads an RTP packet and 'pushes' it to VoiceEngine.
     * Invoked from VoiceEngineNetTask.
     */
    virtual void pushPacket() = 0;

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

    /**
     * Sets the receiving address for media.
     *
     * @param szAddress IP address for receiving media.
     *        For IPV4, a dotted-quad string.
     */
    virtual void setPreferredReceiveAddress(const char* szAddress, int port) = 0;

/* ============================ ACCESSORS ================================= */

    /**
     * Retrieves the last time that data
     * was read from this object.
     */
    virtual const OsTime getLastTimeRead() = 0;


};
#endif // #ifndef _IVoiceEngineMediaSocket_h_