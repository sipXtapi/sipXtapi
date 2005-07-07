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
 * This exception is thown when a user tries to use a locked audio device.  An 
 * audio device becomes locked when an application requests exclusive use of 
 * that resource.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PMediaLockedException extends PMediaException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PMediaLockedException(String description)
    {
        super(description) ;
    }        
    
    
    /** 
     * Constructs an exception without a description message.
     */
    public PMediaLockedException()
    {
        super() ;
    }        
}
