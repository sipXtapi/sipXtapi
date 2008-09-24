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
// $$
//////////////////////////////////////////////////////////////////////////////
// Author: Bob Andreasen
// 

#include <assert.h>
#include <stdlib.h>
#if defined(__pingtel_on_posix__)
#   include <sys/types.h>
#   include <sys/socket.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#elif defined WIN32
#   include <winsock2.h>
#endif

#include "ARS/SnacCodec.h"

//////////////////////////////////////////////////////////////////////////////

SnacCodec::SnacCodec()
{
    mLength = 0 ;
    mnBuf = 256 ;
    mpBuf = (char*) malloc(mnBuf) ;
    memset(mpBuf, 0, mnBuf) ;
    mCursor = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

SnacCodec::SnacCodec(const char* pBuf, int nBuf)
{
    assert(pBuf != NULL) ;
    assert(nBuf > 0) ;

    mCursor = 0 ;

    if (pBuf && nBuf > 0)
    {
        mLength = nBuf ;
        mnBuf = nBuf ;
        mpBuf = (char*) malloc(mnBuf) ;
        assert(mpBuf != NULL) ;
        memcpy(mpBuf, pBuf, mnBuf) ;
    }
    else
    {
        mLength = 0 ;
        mnBuf = 0 ;
        mpBuf = NULL ;
    }
}

//////////////////////////////////////////////////////////////////////////////

SnacCodec::~SnacCodec()
{
    if (mpBuf)
    {
        free(mpBuf) ;
        mpBuf = NULL ;
    }
}

//////////////////////////////////////////////////////////////////////////////

void SnacCodec::rewind()
{
    mCursor = 0 ;
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::appendByte(unsigned char data) 
{
    int size = sizeof(data) ;
    bool bRC = checkSize(size) ;
    if (bRC)
    {
        memcpy(&mpBuf[mLength], &data, size) ;
        mLength += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::getByte(unsigned char& data)
{
    int size = sizeof(data) ;
    bool bRC = checkBounds(mCursor, size) ;
    if (bRC)
    {
        memcpy(&data, &mpBuf[mCursor], size) ;
        mCursor += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::appendShort(unsigned short data) 
{
    int size = sizeof(data) ;

    bool bRC = checkSize(size) ;
    if (bRC)
    {
        data = htons(data) ;
        memcpy(&mpBuf[mLength], &data, size) ;
        mLength += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::getShort(unsigned short& data)
{
    int size = sizeof(data) ;
    bool bRC = checkBounds(mCursor, size) ;
    if (bRC)
    {
        memcpy(&data, &mpBuf[mCursor], size) ;
        mCursor += size ;
        data = ntohs(data) ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::appendLong(unsigned long data) 
{
    int size = sizeof(data) ;

    bool bRC = checkSize(size) ;
    if (bRC)
    {
        data = htonl(data) ;
        memcpy(&mpBuf[mLength], &data, size) ;
        mLength += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::getLong(unsigned long& data)
{
    int size = sizeof(data) ;
    bool bRC = checkBounds(mCursor, size) ;
    if (bRC)
    {
        memcpy(&data, &mpBuf[mCursor], size) ;
        mCursor += size ;
        data = ntohl(data) ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::appendBlob(const unsigned char* data, int length) 
{
    int size = length ;

    bool bRC = checkSize(size) ;
    if (bRC && (size > 0))
    {
        memcpy(&mpBuf[mLength], data, size) ;
        mLength += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::getBlob(unsigned char* data, int length)
{
    int size = length ;
    bool bRC = checkBounds(mCursor, size) ;
    if (bRC)
    {
        memcpy(&data, &mpBuf[mCursor], size) ;
        mCursor += size ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::appendPString(const char* data) 
{
    bool bRC = false ;

    int len = strlen(data) ;
    assert(len <= 255) ;
    if (len <= 255)
    {
        bRC = ( appendByte((unsigned char) len) &&
                appendBlob((const unsigned char*) data, len) ) ;

    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::getPString(char* data)   // 256 byte string len required
{
    unsigned char len = 0 ;
    bool bRC = getByte(len) ;
    if (bRC && len > 0)
    {
        bRC = getBlob((unsigned char*) data, len) ;
        if (bRC)
        {
            data[len] = 0 ;
        }
    }

    return bRC ;
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::updateShort(int position, unsigned short data) 
{
    int size = sizeof(data) ;
    
    data = htons(data) ;        
    bool bRC = checkBounds(position, size) ;
    if (bRC)
    {
        memcpy(&mpBuf[position], &data, size) ;
    }
    return bRC ;        
}

//////////////////////////////////////////////////////////////////////////////

int SnacCodec::getLength() 
{
    return mLength ;
}

//////////////////////////////////////////////////////////////////////////////


const char* SnacCodec::getData() 
{
    return mpBuf ;
}

//////////////////////////////////////////////////////////////////////////////

void SnacCodec::hexdump(UtlString& output)
{
    UtlString temp ;

    for (int i=0; i<mLength; i++)
    {
        temp.format("%02X", (unsigned char) mpBuf[i]) ;
        if (((i+1) % 2) == 0)
            temp.append(" ") ;
        if (((i+1) % 16) == 0)
            temp.append("\n") ;
        output.append(temp) ;
    }
    output.append("\n\n") ;                
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::checkSize(int len)
{
    bool bRC = true ;

    if (mLength + len >= mnBuf)
    {
        int newSize = mnBuf*2 ;
        char* pNewBuf = (char*) realloc(mpBuf, newSize) ;
        if (pNewBuf)
        {
            mpBuf = pNewBuf ;
            mnBuf = newSize ;
        }
        else
            bRC = false ;
    }

    return bRC && mpBuf ;
}

//////////////////////////////////////////////////////////////////////////////

bool SnacCodec::checkBounds(int position, int length)
{
    bool bRC = false ;

    if ((position >= 0) && (position+length <= mLength))
        bRC = true ;

    return bRC && mpBuf ;
}

//////////////////////////////////////////////////////////////////////////////

