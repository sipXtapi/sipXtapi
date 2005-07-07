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
 * This exception is a tag/convenience like PSTAPIException, however this 
 * exception allows developers to catch all call related exceptions and still
 * filter out PMediaException-derived exceptions.
 *
 * @see PSTAPIException
 * @see PMediaException
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PCallException extends PSTAPIException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PCallException(String description)
    {
        super(description) ;
    }        


    /** 
     * Constructs an exception without a description message.
     */
    public PCallException()
    {
        super() ;
    }
    
    
}
