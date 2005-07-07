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
 * This exception is thrown when the media file, stream, or resource is not 
 * available.  For example, if the media manager is given a URL and the server
 * cannot be contacted, or if the media manager was told to enable a missing audio
 * device, this exception applies.
 *
 * @author Robert J. Andreasen, Jr.
 */ 
public class PMediaUnavailableException extends PMediaException
{


    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PMediaUnavailableException(String description)
    {
        super(description) ;
    }        

    
    /** 
     * Constructs an exception without a description message.
     */
    public PMediaUnavailableException()
    {
        super() ;
    }        
}
