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
 * This exception is provided as tag/convenience for developers.  It allows 
 * developers to catch all STAPI exceptions generically, but still separately
 * from other java or 3rd party exceptions.
 *
 * @author Robert J. Andreasen, Jr.
 */
public class PSTAPIException extends Exception
{                
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception.  This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PSTAPIException(String description)
    {
        super(description) ;
    }    

    
    /** 
     * Constructs an exception without a description message.
     */
    public PSTAPIException()
    {
        super() ;        
    }    
    
}
