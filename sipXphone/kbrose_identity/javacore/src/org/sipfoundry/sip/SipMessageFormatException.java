/*
 * $Id: //depot/OPENDEV/sipXphone/javacore/src/org.sipfoundry/sip/SipMessageFormatException.java#2 $
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
 * This exception is thrown if a SipMessage is improperly formed or missing
 * required fields.  For example, omitting the to field would result in a 
 * SipMessageFormatException when attempting to post or send it.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class SipMessageFormatException extends Exception 
{        
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public SipMessageFormatException(String description)
    {
        super(description) ;
    }        
    

    /** 
     * Constructs an exception without a description message.
     */
    public SipMessageFormatException()
    {
        super() ;
    }                
}
