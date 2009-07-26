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

#ifndef _MediaDeviceInfo_h
#define _MediaDeviceInfo_h

/* SYSTEM INCLUDES */
/* APPLICATION INCLUDES */
#include "utl/UtlString.h"

/* DEFINES */
/* MACROS */
/* EXTERNAL FUNCTIONS */
/* EXTERNAL VARIABLES */
/* CONSTANTS */
/* STRUCTS */
/* TYPEDEFS */
/* FORWARD DECLARATIONS */


/**
 *  Storage class for Media Device Information.  This information
 *  is used purely for reporting/diagnositics.
 */
class MediaDeviceInfo
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */

public:
    enum MediaDeviceInfoType
    {
        MDIT_UNKNOWN,       ///< Unknown/Uninialized type
        MDIT_AUDIO_INPUT,   ///< Audio Input/Microphone
        MDIT_AUDIO_OUTPUT,  ///< Audio Output/Speaker
        MDIT_VIDEO_CAPTURE  ///< WebCam/Capture Card/etc
    } ;

/* ============================ CREATORS ================================== */

    MediaDeviceInfo() ;
    virtual ~MediaDeviceInfo() ;
    MediaDeviceInfo(MediaDeviceInfo::MediaDeviceInfoType aType) ;
    MediaDeviceInfo(const MediaDeviceInfo& rMediaDeviceInfo) ;

/* ============================ MANIPULATORS ============================== */

    MediaDeviceInfo& operator=(const MediaDeviceInfo& rMediaDeviceInfo) ;
    void reset() ;

    void setRequested(const char* szRequested) ;
    void setSelected(const char* szSelected) ;
    void setParameters(const char* szParameters) ;
    void appendErrors(const char* szError) ;

/* ============================ ACCESSORS ================================= */

    const char* toString(UtlString& results) const ;
    const UtlString& getRequested() const { return mRequested ; } ;
    const UtlString& getSelected() const { return mSelected ; } ; 
    const UtlString& getParameters() const { return mParameters ; } ; 
    const UtlString& getErrors() const { return mErrors ; } ; 

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    MediaDeviceInfoType mType ; ///< Media Device Type
    UtlString mRequested ;      ///< What did the app request?
    UtlString mSelected ;       ///< What did we end up using?
    UtlString mParameters ;     ///< Any device specific info
    UtlString mErrors ;         ///< Any device errors

/* //////////////////////////// PRIVATE /////////////////////////////////// */

} ;

#endif // _MediaDeviceInfo_h
