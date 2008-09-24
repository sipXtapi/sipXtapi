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

#ifndef SNACCODEC_H
#define SNACCODEC_H

#include "utl/UtlString.h"


//////////////////////////////////////////////////////////////////////////////


class SnacCodec
{
protected:
    int   mLength ;     // length of accumulated data
    int   mnBuf ;       // size of internal buffer
    char* mpBuf ;       // interal buffer
    int   mCursor ;     // cursor for parsing

public:
    SnacCodec() ;
    SnacCodec(const char* pBuf, int nBuf) ;
    virtual ~SnacCodec() ;

    bool appendByte(unsigned char data) ;
    bool getByte(unsigned char& data) ;

    bool appendShort(unsigned short data) ;
    bool getShort(unsigned short& data) ;
    bool updateShort(int position, unsigned short data)  ;

    bool appendLong(unsigned long data) ;
    bool getLong(unsigned long& data) ;

    bool appendBlob(const unsigned char* data, int length)  ;
    bool getBlob(unsigned char* data, int length) ;

    bool appendPString(const char* data)  ;
    bool getPString(char* data) ;   // 256 byte string len required

    
    void rewind() ;
    int getLength() ;
    const char* getData() ;

    void hexdump(UtlString& output) ;

protected:
    bool checkSize(int len) ;
    bool checkBounds(int position, int length) ;
} ;

//////////////////////////////////////////////////////////////////////////////

#endif

