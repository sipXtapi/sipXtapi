/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipTimeoutException.java#2 $
 *
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $$
 */
 
package org.sipfoundry.sip ;

import java.lang.Exception ;


/**
 * This exception is thrown when an Request is sent to a remote host and
 * no final response is received for an extended time period.
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public class SipTimeoutException extends Exception 
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public SipTimeoutException(String description)
    {
        super(description) ;
    }        
    

    /** 
     * Constructs an exception without a description message.
     */
    public SipTimeoutException()
    {
        super() ;
    }           
}
