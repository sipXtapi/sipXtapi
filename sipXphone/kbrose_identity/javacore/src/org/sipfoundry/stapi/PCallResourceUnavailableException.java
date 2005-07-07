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
 * This exception is thrown when an action or request could not be completed 
 * because of a resource limitation. For example, this exception occurs when  
 * an attempt is made to add another conference member when the maximum number 
 * of simultaneous connections has already been reached.
 * 
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PCallResourceUnavailableException extends PCallException
{        
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */        
    public PCallResourceUnavailableException(String description)
    {
        super(description) ;
    }

    
    /** 
     * Constructs an exception without a description message.
     */
    public PCallResourceUnavailableException()
    {
        super() ;
    }    
}
