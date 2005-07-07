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
 * This exception is thrown when supplied media is invalid, incompatible, or 
 * explicitly unsupported.  For example, this exception applies when a request
 * is made to the media manger to play an unsupported audio format.
 *
 * @author Robert J. Andreasen, Jr. 
 */
public class PMediaNotSupportedException extends PMediaException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PMediaNotSupportedException(String description)
    {
        super(description) ;
    }        
    
    
    /** 
     * Constructs an exception without a description message.
     */
    public PMediaNotSupportedException()
    {
        super() ;
    }
}
