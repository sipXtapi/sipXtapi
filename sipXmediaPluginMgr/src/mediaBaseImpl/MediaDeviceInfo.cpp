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
/* APPLICATION INCLUDES */
#include "mediaBaseImpl/MediaDeviceInfo.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo::MediaDeviceInfo()
{
    mType = MediaDeviceInfo::MDIT_UNKNOWN ;
}

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo::~MediaDeviceInfo()
{
}

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo::MediaDeviceInfo(MediaDeviceInfo::MediaDeviceInfoType aType)
{
    mType = aType ;
}

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo::MediaDeviceInfo(const MediaDeviceInfo& rMediaDeviceInfo)
{
    mType        = rMediaDeviceInfo.mType;
    mRequested   = rMediaDeviceInfo.mRequested;
    mSelected    = rMediaDeviceInfo.mSelected;
    mParameters  = rMediaDeviceInfo.mParameters;
    mErrors      = rMediaDeviceInfo.mErrors;
}

//////////////////////////////////////////////////////////////////////////////

MediaDeviceInfo& 
MediaDeviceInfo::operator=(const MediaDeviceInfo& rMediaDeviceInfo)
{
    if (this == &rMediaDeviceInfo)
        return *this;

    mType        = rMediaDeviceInfo.mType;
    mRequested   = rMediaDeviceInfo.mRequested;
    mSelected    = rMediaDeviceInfo.mSelected;
    mParameters  = rMediaDeviceInfo.mParameters;
    mErrors      = rMediaDeviceInfo.mErrors;

    return *this;
}

//////////////////////////////////////////////////////////////////////////////

void MediaDeviceInfo::reset()
{
    mRequested.remove(0) ;
    mSelected.remove(0) ;
    mParameters.remove(0) ;
    mErrors.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaDeviceInfo::setRequested(const char* szRequested) 
{
    if (szRequested)
        mRequested = szRequested ;
    else
        mRequested.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaDeviceInfo::setSelected(const char* szSelected) 
{
    if (szSelected)
        mSelected = szSelected ;
    else
        mSelected.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaDeviceInfo::setParameters(const char* szParameters) 
{
    if (szParameters)
        mParameters = szParameters ;
    else
        mParameters.remove(0) ;
}

//////////////////////////////////////////////////////////////////////////////

void MediaDeviceInfo::appendErrors(const char* szError) 
{
    if (szError)
    {
        mErrors.append(szError) ;
        mErrors.append("\n") ;
    }
}

//////////////////////////////////////////////////////////////////////////////

const char* MediaDeviceInfo::toString(UtlString& results) const
{
    switch (mType)
    {
        case MDIT_AUDIO_INPUT:
            results = "AUDIO_INPUT" ;
            break ;
        case MDIT_AUDIO_OUTPUT:
            results = "AUDIO_OUTPUT" ;
            break ;
        case MDIT_VIDEO_CAPTURE:
            results = "VIDEO_CAPTURE" ;
            break ;
        default:
            results = "UNKNOWN" ;
            break ;
    }

    results.append(" requested=") ;
    results.append(mRequested) ;
    results.append(" used=") ;
    results.append(mSelected) ;
    results.append(" params=") ;
    results.append(mParameters) ;
    results.append(" errors=") ;
    results.append(mErrors) ;

    return results.data() ;
}


