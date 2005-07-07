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
 * This exception is thrown when an action or request contradicts the current 
 * call state. Examples include trying to place a held call on hold a 
 * second time, or trying to disconnect a non-existent conference member.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PCallStateException extends PCallException
{
    /** 
     * Constructs an exception without a description message.
     */
    public PCallStateException()
    {
        super() ;
    }

    
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */                
    public PCallStateException(String description)
    {
        super(description) ;
    }        
}
