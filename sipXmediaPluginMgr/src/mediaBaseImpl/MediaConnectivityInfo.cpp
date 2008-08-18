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

/* SYSTEM INCLUDES */
#include <assert.h>

/* APPLICATION INCLUDES */
#include "mediaBaseImpl/MediaConnectivityInfo.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//////////////////////////////////////////////////////////////////////////////

MediaConnectivityInfo::MediaConnectivityInfo()
{
    mType = MCIT_UNKNOWN ;
    reset() ;
}

//////////////////////////////////////////////////////////////////////////////

MediaConnectivityInfo::MediaConnectivityInfo(MediaConnectivityInfo::MediaConnectivityInfoType type) 
{
    mType = type ;
    reset() ;
}

//////////////////////////////////////////////////////////////////////////////

MediaConnectivityInfo::MediaConnectivityInfo(const MediaConnectivityInfo& rMediaConnectivityInfo)
{
    mType = rMediaConnectivityInfo.mType ;

    mStunServer = rMediaConnectivityInfo.mStunServer ;
    mTurnServer = rMediaConnectivityInfo.mTurnServer ;
    mArsServer = rMediaConnectivityInfo.mArsServer ;
    mArsHttpsProxy = rMediaConnectivityInfo.mArsHttpsProxy ;
    mbUPNP = rMediaConnectivityInfo.mbUPNP ;
    mNumLocalCandidates = rMediaConnectivityInfo.mNumLocalCandidates ;
    for (int i=0; i<MAX_LOCAL_CANDIDATES; i++)
        mLocalCandidates[i] = rMediaConnectivityInfo.mLocalCandidates[i] ;
    mOurRelayUsed = rMediaConnectivityInfo.mOurRelayUsed ;
    mRemoteMediaIP = rMediaConnectivityInfo.mRemoteMediaIP ;    
    mRemoteMediaPort = rMediaConnectivityInfo.mRemoteMediaPort ;
    mbIce = rMediaConnectivityInfo.mbIce ;
    mbIceSelectionMS = rMediaConnectivityInfo.mbIceSelectionMS ;
}

//////////////////////////////////////////////////////////////////////////////

MediaConnectivityInfo& 
MediaConnectivityInfo::operator=(const MediaConnectivityInfo& rMediaConnectivityInfo)
{
    if (this == &rMediaConnectivityInfo)
        return *this;

    mType = rMediaConnectivityInfo.mType ;
    mStunServer = rMediaConnectivityInfo.mStunServer ;
    mTurnServer = rMediaConnectivityInfo.mTurnServer ;
    mArsServer = rMediaConnectivityInfo.mArsServer ;
    mArsHttpsProxy = rMediaConnectivityInfo.mArsHttpsProxy ;
    mbUPNP = rMediaConnectivityInfo.mbUPNP ;
    mNumLocalCandidates = rMediaConnectivityInfo.mNumLocalCandidates ;
    for (int i=0; i<MAX_LOCAL_CANDIDATES; i++)
        mLocalCandidates[i] = rMediaConnectivityInfo.mLocalCandidates[i] ;
    mOurRelayUsed = rMediaConnectivityInfo.mOurRelayUsed ;
    mRemoteMediaIP = rMediaConnectivityInfo.mRemoteMediaIP ;    
    mRemoteMediaPort = rMediaConnectivityInfo.mRemoteMediaPort ;
    mbIce = rMediaConnectivityInfo.mbIce ;
    mbIceSelectionMS = rMediaConnectivityInfo.mbIceSelectionMS ;

    return *this;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::reset()
{
    mStunServer.remove(0) ;
    mTurnServer.remove(0) ;
    mArsServer.remove(0) ;
    mArsHttpsProxy.remove(0) ;
    mbUPNP = false ;
    mNumLocalCandidates = 0 ;
    for (int i=0; i<MAX_LOCAL_CANDIDATES; i++)
        mLocalCandidates[i].remove(0) ; 
    mOurRelayUsed = MCIRT_NONE ;
    mRemoteMediaIP.remove(0) ;    
    mRemoteMediaPort = 0 ; 
    mbIce = false ;
    mbIceSelectionMS = -1 ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setStunServer(const char* szStunServer) 
{
    if (szStunServer)
        mStunServer = szStunServer ;
    else
        mStunServer.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& MediaConnectivityInfo::getStunServer() const 
{
    return mStunServer ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setTurnServer(const char* szTurnServer) 
{
    if (szTurnServer)
        mTurnServer = szTurnServer ;
    else
        mTurnServer.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& MediaConnectivityInfo::getTurnServer() const 
{
    return mTurnServer ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setArsServer(const char* szArsServer) 
{
    if (szArsServer)
        mArsServer = szArsServer ;
    else
        mArsServer.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& MediaConnectivityInfo::getArsServer() const 
{
    return mArsServer ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setArsHttpsProxy(const char* szArsHttpsProxy) 
{
    if (szArsHttpsProxy)
        mArsHttpsProxy = szArsHttpsProxy ;
    else
        mArsHttpsProxy.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

const UtlString& MediaConnectivityInfo::getArsHttpsProxy() const 
{
    return mArsHttpsProxy ;
}


//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setUPNP(bool bUPNP) 
{
    mbUPNP = bUPNP ;
}

//////////////////////////////////////////////////////////////////////////////

bool MediaConnectivityInfo::getUPNP() const 
{
    return mbUPNP ;
}

//////////////////////////////////////////////////////////////////////////////

bool MediaConnectivityInfo::addLocalCandidate(const char* szType, 
                                              const char* szIP, 
                                              int port) 
{
    assert(szType != NULL) ;
    assert(szIP != NULL) ;

    bool bRC = false ;
    if (mNumLocalCandidates < MAX_LOCAL_CANDIDATES)
    {
        UtlString candidate ;
        char      cTemp[32] ;

        // Format candidate as "type:ip:port"
        if (szType)
            candidate.append(szType) ;
        candidate.append(':') ;
        if (szIP)
            candidate.append(szIP) ;
        candidate.append(':') ;
        candidate.append(itoa(port, cTemp, 10)) ;

        mLocalCandidates[mNumLocalCandidates] = candidate ;
        
        bRC = true ;

        mNumLocalCandidates++ ;
    }
    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

size_t MediaConnectivityInfo::getNumLocalCandidates() const 
{
    return mNumLocalCandidates ;
}

//////////////////////////////////////////////////////////////////////////////

const char* MediaConnectivityInfo::getLocalCandidate(size_t index) const 
{
    const char* szRC = NULL ;

    if (index >= 0 && index < mNumLocalCandidates)
    {
        szRC = mLocalCandidates[index].data() ;
    }

    return szRC ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setOurRelayType(MediaConnectivityInfoRelayType relayType) 
{
    mOurRelayUsed = relayType ;
}

//////////////////////////////////////////////////////////////////////////////

MediaConnectivityInfo::MediaConnectivityInfoRelayType MediaConnectivityInfo::getOurRelayType() const 
{
    return mOurRelayUsed ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setRemoteMediaAddr(const char* szIP, int port) 
{
    if (szIP)
        mRemoteMediaIP = szIP ;
    else
        mRemoteMediaIP.remove(0) ;
    mRemoteMediaPort = port ;

}

//////////////////////////////////////////////////////////////////////////////

const UtlString& MediaConnectivityInfo::getRemoteMediaIP() const 
{
    return mRemoteMediaIP ;
}

//////////////////////////////////////////////////////////////////////////////

int MediaConnectivityInfo::getRemoteMediaPort() const 
{
    return mRemoteMediaPort ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setIce(bool bIce) 
{
    mbIce = bIce ;
}

//////////////////////////////////////////////////////////////////////////////

bool MediaConnectivityInfo::getIce() const 
{
    return mbIce ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaConnectivityInfo::setIceTime(int timeInMS) 
{
    mbIceSelectionMS = timeInMS ;
}

//////////////////////////////////////////////////////////////////////////////

int MediaConnectivityInfo::getIceTime() const 
{
    return mbIceSelectionMS ;
}

//////////////////////////////////////////////////////////////////////////////

const char* MediaConnectivityInfo::toString(UtlString& results) const
{
    UtlString localCandidates ;

    for (size_t i=0; i<mNumLocalCandidates; i++)
    {
        localCandidates.append(mLocalCandidates[i]) ;
        localCandidates.append('\n') ;
    }

    results.format("Type=%s stun=%s turn=%s ars=%s arshttps=%s\n" \
            "Local Candidates=\n%s" \
            "UPNP=%d ourRelay=%s, remoteIP=%s:%d ice=%d iceTime=%d\n",
            getTypeString(mType),
            mStunServer.data(),
            mTurnServer.data(),
            mArsServer.data(),
            mArsHttpsProxy.data(),
            localCandidates.data(),
            mbUPNP,
            getRelayTypeString(mOurRelayUsed),
            mRemoteMediaIP.data(),
            mRemoteMediaPort,
            mbIce,
            mbIceSelectionMS) ;

    return results.data() ;
}

//////////////////////////////////////////////////////////////////////////////

const char* MediaConnectivityInfo::getTypeString(MediaConnectivityInfoType type)
{
    const char* szRC = "UNKNOWN_ERROR" ;
    

    switch (type)
    {
    case MCIT_UNKNOWN:
        szRC = "UNKNOWN" ;
        break ;
    case MCIT_AUDIO_RTP:
        szRC = "AUDIO_RTP" ;
        break ;
    case MCIT_AUDIO_RTCP:
        szRC = "AUDIO_RTCP" ;
        break ;
    case MCIT_VIDEO_RTP:
        szRC = "VIDEO_RTP" ;
        break ;
    case MCIT_VIDEO_RTCP:
        szRC = "VIDEO_RTCP" ;
        break ;
    default:
        assert(false) ; // Bogus Value
        break ;
    }

    return szRC ;
}

//////////////////////////////////////////////////////////////////////////////

const char* MediaConnectivityInfo::getRelayTypeString(MediaConnectivityInfoRelayType type) 
{
    const char* szRC = "UNKNOWN_ERROR" ;

    switch (type)
    {
    case MCIRT_NONE:
        szRC = "NONE" ;
        break ;
    case MCIRT_TURN_UDP:
        szRC = "TURN_UDP" ;
        break ;
    case MCIRT_TURN_TCP:
        szRC = "TURN_TCP" ;
        break ;
    case MCIRT_TURN_TLS:
        szRC = "TURN_TLS" ;
        break ;
    case MCIRT_ARS:
        szRC = "MCIRT_ARS" ;
        break ;
    case MCIRT_ARS_HTTP:
        szRC = "ARS_HTTP" ;
        break ;
    case MCIRT_ARS_HTTPS:
        szRC = "ARS_HTTPS" ;
        break ;
    default:
        assert(false) ; // Bogus Value
        break ;
    }

    return szRC ;
}

//////////////////////////////////////////////////////////////////////////////
