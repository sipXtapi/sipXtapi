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

#ifndef _CpMediaInStream_h_
#define _CpMediaInStream_h_

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include "mediaInterface/IMediaInterface.h"
#include "os/OsMutex.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class IMediaEventListener ;
enum IMediaEvent_DeviceTypes ;

class CpMediaInStream : public IMediaEventEmitter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor taking a buffer
     */
    CpMediaInStream(const char* cData, 
                              size_t nBytes, 
                              bool bRepeat, 
                              void *pSource,
                              IMediaEventListener* pListener, 
                              IMediaEvent_DeviceTypes type);

    /**
     * Default constructor taking a file
     */ 
    CpMediaInStream(const char* szFilename, 
                              bool bRepeat, 
                              void *pSource,
                              IMediaEventListener* pListener, 
                              IMediaEvent_DeviceTypes type);
     
    /**
     * Destructor
     */
    virtual ~CpMediaInStream();

/* ============================ MANIPULATORS ============================== */
	virtual int doRead(void *buf,int len);
	// len - size in bytes that should be read
	// returns the size in bytes read (=len before end and =[0..len-1] at end similar to fread)

	// Called when a wav-file needs to be rewinded
	virtual int doRewind();

    virtual void close() ;

    virtual void onListenerRemoved();
   
/* ============================ ACCESSORS ================================= */

    virtual const void* getSource() const ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */    
protected:
    FILE*  mpFile ;
    char*  mcData ;
    size_t mnBytes ;
    size_t mPosition ;
    bool   mbRepeat ;
    IMediaEventListener* mpListener ;
    IMediaEvent_DeviceTypes mType ;
    OsMutex mLock ;
    void*   mpSource ;

    int doBufferRead(void* pBuf, int len) ;
    int doFileRead(void* pBuf, int len) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor */
    CpMediaInStream(const CpMediaInStream& rInStream);     

    /** Disabled equals operator*/
    CpMediaInStream& operator=(const CpMediaInStream& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoiceEngineBufferInStream_h_

