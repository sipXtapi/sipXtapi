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

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>

// APPLICATION INCLUDES
#include "include/VoiceEngineBufferInStream.h"
#include "mediaInterface/IMediaInterface.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
VoiceEngineBufferInStream::VoiceEngineBufferInStream(const char* cData, 
                                                     size_t      nBytes, 
                                                     bool        bRepeat,
                                                     void *      pSource,
                                                     IMediaEventListener* pListener,
                                                     IMediaEvent_DeviceTypes type)
    : CpMediaInStream(cData,
                      nBytes,
                      bRepeat,
                      pSource,
                      pListener,
                      type)
{
}

// Constructor
VoiceEngineBufferInStream::VoiceEngineBufferInStream(const char* szFilename, 
                                                     bool bRepeat,
                                                     void *pSource,
                                                     IMediaEventListener* pListener,
                                                     IMediaEvent_DeviceTypes type)
    : CpMediaInStream(szFilename,
        bRepeat,
        pSource,
        pListener,
        type)
{
}


// Destructor
VoiceEngineBufferInStream::~VoiceEngineBufferInStream()
{
}


/* ============================ MANIPULATORS ============================== */

int VoiceEngineBufferInStream::Read(void *pBuf, int len) 
{
    return doRead(pBuf, len) ;
}

void VoiceEngineBufferInStream::onListenerRemoved()
{
    OsLock lock(mLock);
    mpListener = NULL;
}

void VoiceEngineBufferInStream::close() 
{
    OsLock lock(mLock) ;

    if (mcData != NULL)
    {
        free(mcData) ;
        mcData = NULL ;
        mnBytes = 0 ;
        mPosition = 0 ;
        mbRepeat = false ;

        if (mpListener != NULL)
        {
            mpListener->onBufferStop(mType) ;
            mpListener = NULL ;
        }
    }
    else if (mpFile != NULL)
    {
        fclose(mpFile) ;
        mpFile = NULL ;
        mnBytes = 0 ;
        mPosition = 0 ;
        mbRepeat = false ;

        if (mpListener != NULL)
        {
            mpListener->onFileStop(mType) ;
            mpListener = NULL ;
        }
    }
}


int VoiceEngineBufferInStream::Rewind() 
{
    doRewind() ;
    return 0 ;
}


/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


