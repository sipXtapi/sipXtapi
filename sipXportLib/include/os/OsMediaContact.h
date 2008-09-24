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
// Author: Bob Andreasen
//

#ifndef _OsMediaContact_h_
#define _OsMediaContact_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlContainable.h"
#include "utl/UtlString.h"
#include "os/OsSocket.h"
#include "os/OsContact.h"
#include "os/IOsNatSocket.h"    // for RTP_TRANSPORT

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* ============================ ENUMERATIONS ============================== */
/**
 * OsMediaContact is a an extension of an OsContact adding an rtcp port and
 * RTP transport type.
 */
class OsMediaContact : public OsContact
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ FRIENDS =================================== */
    
/* ============================ ENUMERATIONS ============================== */

/* ============================ CREATORS ================================== */

    /**
     * Constructor
     */
    OsMediaContact(const char* szAddress,
                   int rtpPort,
                   int rtcpPort,
                   OsSocket::IpProtocolSocketType protocol,
                   RTP_TRANSPORT transportType,
                   IpAddressType addressType = IP4);

    /**
     * Default Constructor
     */
    OsMediaContact() ;
                       
    /**
     * Copy constructor
     */                       
    OsMediaContact(const OsMediaContact& ref);

    /**
     * Assignment operator
     */
    OsMediaContact& operator=(const OsMediaContact& ref) ;
         
    /**
     * Destructor
     */
    virtual ~OsMediaContact();


/* ============================ MANIPULATORS ============================== */
    
    void setRtcpPort(int port) ;

    void setTransportType(RTP_TRANSPORT transportType) ;

/* ============================ ACCESSORS ================================= */
    
    /**
     * Accessor for the port value.
     */
    int getRtcpPort() const;

    RTP_TRANSPORT getTransportType() const ;
    
    /**
     * Calculate a unique hash code for this object.  If the equals
     * operator returns true for another object, then both of those
     * objects must return the same hashcode.
     */
    virtual unsigned int hash() const ;

    /**
     * Get the ContainableType for a UtlContainable derived class.
     */
    virtual UtlContainableType getContainableType() const;
    
/* ============================ INQUIRY =================================== */

    /**
     * Compare the this object to another like-objects.  Results for 
     * designating a non-like object are undefined.
     *
     * @returns 0 if equal, < 0 if less then and >0 if greater.
     */
    virtual int compareTo(UtlContainable const *) const ;    

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    static UtlContainableType TYPE ;    /** < Class type used for runtime checking */ 

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:    
    int mRtcpPort ;                 /**< RTCP port */
    RTP_TRANSPORT mTransportType ;  /**< Transport type */

} ;

/* ============================ INLINE METHODS ============================ */

#endif    // _OsMediaContact_h_
