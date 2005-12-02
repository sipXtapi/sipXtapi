/*
 * $Id$
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.stapi ;

import java.lang.* ;


/**
 * This exception is thrown when an invalid or unsupported address is passed 
 * in to one of the STAPI methods.  For example, this exception could be 
 * thrown when an unknown protocol is specified in the address 
 * ("protocolXYZ:user@domain.com" instead of "sip:user@domain.com").
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public class PCallAddressException extends PCallException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PCallAddressException(String description)
    {
        super(description) ;
    }        
    

    /** 
     * Constructs an exception without a description message.
     */
    public PCallAddressException()
    {
        super() ;
    }        
}
