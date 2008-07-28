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
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>

// APPLICATION INCLUDES
#include "mediaBaseImpl/CpMediaInStream.h"
#include "mediaInterface/IMediaInterface.h"
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CpMediaInStream::CpMediaInStream(const char* cData, 
                                                     size_t      nBytes, 
                                                     bool        bRepeat,
                                                     void *      pSource,
                                                     IMediaEventListener* pListener,
                                                     IMediaEvent_DeviceTypes type)
    : mLock(OsMutex::Q_FIFO)
{
    bool bError = true ;

    mcData = NULL ;
    mpFile = NULL ;
    mnBytes = 0 ;
    mPosition = 0 ;
    mbRepeat = false ;
    mpSource = pSource ;

    if (pListener)
    {
        pListener->onListenerAddedToEmitter(this);
    }
    mpListener = pListener ;
    mType = type ;    

    if (mpListener != NULL)
    {
        mpListener->onBufferStart(mType) ;
    }

    if (cData && nBytes > 0)
    {
        mcData = (char*) malloc(nBytes) ;
        if (mcData)
        {
            memcpy(mcData, cData, nBytes) ;
            mnBytes = nBytes ;
            mbRepeat = bRepeat ;
            bError = false ;
        }
    }

    if (bError)
    {
        if (mpListener != NULL)
        {
            // TODO:: Give cause code/error indication
            mpListener->onBufferStop(mType) ;
        }
    }
}

// Constructor
CpMediaInStream::CpMediaInStream(const char* szFilename, 
                                                     bool bRepeat,
                                                     void *pSource,
                                                     IMediaEventListener* pListener,
                                                     IMediaEvent_DeviceTypes type)
    : mLock(OsMutex::Q_FIFO)
{
    bool bError = true ;

    mcData = NULL ;
    mpFile = NULL ;
    mnBytes = 0 ;
    mPosition = 0 ;
    mbRepeat = false ;
    mpSource = pSource ;
    mpListener = pListener ;
    mType = type ;

    if (mpListener != NULL)
    {
        mpListener->onFileStart(mType) ;
    }

    mpFile = fopen(szFilename, "rb") ;
    if (mpFile)
    {
        fseek(mpFile, 0, SEEK_END) ;
        mnBytes = ftell(mpFile) ;
        fseek(mpFile, 0, SEEK_SET) ;
        mbRepeat = bRepeat ;
        bError = false ;
    }

    if (bError)
    {
        if (mpListener != NULL)
        {
            mpListener->onFileStop(mType) ;
        }
    }
}


// Destructor
CpMediaInStream::~CpMediaInStream()
{
    close() ;
}


/* ============================ MANIPULATORS ============================== */

int CpMediaInStream::doRead(void *pBuf, int len) 
{
    int nBytesRead = -1 ;

    assert(len >= 0) ;
    assert(pBuf != NULL) ;

    if (pBuf && len > 0)
    {
        memset(pBuf, 0, len) ;

        if (mcData != NULL)
        {
            nBytesRead = doBufferRead(pBuf, len) ;
        }
        else if (mpFile != NULL)
        {
            nBytesRead = doFileRead(pBuf, len) ;
        }

        if (nBytesRead != len)
        {
            close() ;
        }
    }
    else if (len == 0)
    {
        doRewind() ;
        nBytesRead = 0 ; 
    }

/*
    FILE* fp = fopen("C:\\InStreamLog.txt", "at") ;
    if (fp)
    {
        int bytes = nBytesRead ;
        fprintf(fp, "Read (%p, %d), returned %d", pBuf, len, bytes) ;
        for (int i=0; i<bytes; i++)
        {
            if ((i % 32) == 0)
                fprintf(fp, "\r\n\t") ;

            fprintf(fp, "%02X", (unsigned char) ((unsigned char*) pBuf)[i]) ;            
        }
        fprintf(fp, "\r\n") ;

        fclose(fp) ;
    }
*/

    return nBytesRead ;
}

void CpMediaInStream::onListenerRemoved()
{
    OsLock lock(mLock);
    mpListener = NULL;
}

void CpMediaInStream::close() 
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



/* ============================ ACCESSORS ================================= */

const void* CpMediaInStream::getSource() const 
{
    return mpSource ;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

int CpMediaInStream::doBufferRead(void* pBuf, int len) 
{
    OsLock lock(mLock) ;
    int nBytesRead = 0 ;
  
    if (pBuf && len > 0)
    {
        nBytesRead = mnBytes - mPosition ;
        if (len < nBytesRead)
        {
            nBytesRead = len ;
        }

        memcpy(pBuf, &mcData[mPosition], nBytesRead) ;
        mPosition += nBytesRead ;

        // KNOWN BUG: This will only work if frame size (len) is less then buffer size
        if (mbRepeat && (nBytesRead < len) && (nBytesRead < mnBytes))
        {
            mPosition = 0 ;
            memset(&((char*) pBuf)[nBytesRead], 0, (len-nBytesRead)) ;
            nBytesRead = len ;           
        }
    }

    return nBytesRead ;
}

int CpMediaInStream::doFileRead(void* pBuf, int len) 
{
    OsLock lock(mLock) ;
    int nBytesRead = 0 ;
  
    if (pBuf && len > 0)
    {
        nBytesRead = mnBytes - mPosition ;
        if (len < nBytesRead)
        {
            nBytesRead = len ;
        }

        fread(pBuf, 1, nBytesRead, mpFile) ;
        mPosition += nBytesRead ;

        // KNOWN BUG: This will only work if frame size (len) is less then buffer size
        if (mbRepeat && (nBytesRead < len) && (nBytesRead < mnBytes))
        {
            mPosition = 0 ;
            fseek(mpFile, 0, SEEK_SET) ;

            memset(&((char*) pBuf)[nBytesRead], 0, (len-nBytesRead)) ;
            nBytesRead = len ;           
        }
    }

    return nBytesRead ;
}

int CpMediaInStream::doRewind()
{
    mPosition = 0 ;
    if (mpFile != NULL)
    {
        mPosition = fseek(mpFile, 0, SEEK_SET) ;
    }
    return mPosition;
}



/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


