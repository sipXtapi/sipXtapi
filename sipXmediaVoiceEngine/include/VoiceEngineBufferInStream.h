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

#ifndef _VoiceEngineBufferInStream_h_
#define _VoiceEngineBufferInStream_h_

// SYSTEM INCLUDES
#include <stdio.h>

// APPLICATION INCLUDES
#include "mediaInterface/IMediaInterface.h"
#include "mediaBaseImpl/CpMediaInStream.h"
#include "include/VoiceEngineDefs.h"
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

/**
 * Class short description which ends at the first period found within 
 * the sentence -- even if that spans multiple lines.  
 *
 * Class long description starts immediately after the first period, 
 * however, should be separated by an empty line.  This comment can 
 * also span any number of rows and may include <b>html markup<b>, 
 * however, please make sure that the text is still readable without an
 * html browser.
 */
class VoiceEngineBufferInStream : public InStream, public CpMediaInStream
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor taking a buffer
     */
    VoiceEngineBufferInStream(const char* cData, 
                              size_t nBytes, 
                              bool bRepeat, 
                              void *pSource,
                              IMediaEventListener* pListener, 
                              IMediaEvent_DeviceTypes type);

    /**
     * Default constructor taking a file
     */ 
    VoiceEngineBufferInStream(const char* szFilename, 
                              bool bRepeat, 
                              void *pSource,
                              IMediaEventListener* pListener, 
                              IMediaEvent_DeviceTypes type);
     
    /**
     * Destructor
     */
    virtual ~VoiceEngineBufferInStream();

/* ============================ MANIPULATORS ============================== */
    virtual int Read(void *pBuf, int len); 
	// len - size in bytes that should be read
	// returns the size in bytes read (=len before end and =[0..len-1] at end similar to fread)
    virtual int Rewind() ;

    virtual void close() ;

    virtual void onListenerRemoved();
   
/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */    
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor */
    VoiceEngineBufferInStream(const VoiceEngineBufferInStream& rVoiceEngineBufferInStream);     

    /** Disabled equals operator*/
    VoiceEngineBufferInStream& operator=(const VoiceEngineBufferInStream& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _VoiceEngineBufferInStream_h_
