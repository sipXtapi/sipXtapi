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
// Copyright (C) 2008 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Bob Andreasen (bandreasen AT bpsoft DOT com)

#ifndef _MediaConnectivityInfo_h
#define _MediaConnectivityInfo_h

/* SYSTEM INCLUDES */
/* APPLICATION INCLUDES */
#include "utl/UtlString.h"

/* DEFINES */
#define MAX_LOCAL_CANDIDATES    8

/* MACROS */
/* EXTERNAL FUNCTIONS */
/* EXTERNAL VARIABLES */
/* CONSTANTS */
/* STRUCTS */
/* TYPEDEFS */
/* FORWARD DECLARATIONS */


/**
 *  Storage class for Media Connectivity Information.  This information
 *  is used purely for reporting/diagnositics.
 */
class MediaConnectivityInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    enum MediaConnectivityInfoType
    {
        MCIT_UNKNOWN,     ///< Unknown/Uninialized type
        MCIT_AUDIO_RTP,   ///< audio RTP
        MCIT_AUDIO_RTCP,  ///< audio RTCP
        MCIT_VIDEO_RTP,   ///< video RTP
        MCIT_VIDEO_RTCP,  ///< video RTCP
    } ;

    enum MediaConnectivityInfoRelayType
    {
        MCIRT_NONE,      ///< No Relay used (direct connection)
        MCIRT_TURN_UDP,  ///< TURN UDP relay selected
        MCIRT_TURN_TCP,  ///< Turn TCP relay selected
        MCIRT_TURN_TLS,  ///< TURN TLS relay selected
        MCIRT_ARS,       ///< ARS relay (direct) selected
        MCIRT_ARS_HTTP,  ///< ARS relay via http
        MCIRT_ARS_HTTPS  ///< ARS relay via https
    } ;

public:

/* ============================ CREATORS ================================== */

    MediaConnectivityInfo() ;
    MediaConnectivityInfo(MediaConnectivityInfo::MediaConnectivityInfoType type) ;
    MediaConnectivityInfo(const MediaConnectivityInfo& rMediaConnectivityInfo) ;

/* ============================ MANIPULATORS ============================== */

    MediaConnectivityInfo& operator=(const MediaConnectivityInfo& rMediaConnectivityInfo) ;

    void reset() ;

    void setStunServer(const char* szStunServer) ;
    void setTurnServer(const char* szTurnServer) ;
    void setArsServer(const char* szArsServer) ;
    void setArsHttpsProxy(const char* szArsHttpsProxy) ;
    void setUPNP(bool bUPNP) ;
    bool addLocalCandidate(const char* szType, const char* szIP, int port) ;
    void setOurRelayType(MediaConnectivityInfoRelayType relayType) ;
    void setRemoteMediaAddr(const char* szIP, int port) ;
    void setIce(bool bICE) ;
    void setIceTime(int timeInMS) ;

/* ============================ ACCESSORS ================================= */

    const char* toString(UtlString& results) const ;

    const UtlString& getStunServer() const ;
    const UtlString& getTurnServer() const ;
    const UtlString& getArsServer() const ;
    const UtlString& getArsHttpsProxy() const ;
    bool getUPNP() const ;
    size_t getNumLocalCandidates() const ;
    const char* getLocalCandidate(size_t index) const ;
    MediaConnectivityInfoRelayType getOurRelayType() const ;
    const UtlString& getRemoteMediaIP() const ;
    int getRemoteMediaPort() const ;
    bool getIce() const ;
    int getIceTime() const ;

/* ============================= STATICS ================================== */

    static const char* getTypeString(MediaConnectivityInfoType type);
    static const char* getRelayTypeString(MediaConnectivityInfoRelayType type) ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    MediaConnectivityInfoType mType ;
    UtlString mStunServer ;
    UtlString mTurnServer ;
    UtlString mArsServer ;
    UtlString mArsHttpsProxy ;
    bool mbUPNP ;

    size_t    mNumLocalCandidates ;
    UtlString mLocalCandidates[MAX_LOCAL_CANDIDATES] ; // type:ip:port

    MediaConnectivityInfoRelayType mOurRelayUsed ;

    UtlString mRemoteMediaIP ;
    int       mRemoteMediaPort ;
    
    bool mbIce ;
    int  mbIceSelectionMS ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */

} ;

#endif // _MediaDeviceInfo_h
