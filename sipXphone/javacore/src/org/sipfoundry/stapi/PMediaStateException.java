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
 * This exception type is thrown when an action or request contradicts the 
 * current media state; for example, muting an audio device that is 
 * already muted. 
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PMediaStateException extends PMediaException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception.  This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */    
    public PMediaStateException(String description)
    {
        super(description) ;
    }        


    /** 
     * Constructs an exception without a description message.
     */
    public PMediaStateException()
    {
        super() ;
    }
    
}
