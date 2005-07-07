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
 * This exception is thrown when an action or request is not supported.  
 * Deciding whether an action is supported is often a runtime decision
 * and not limited to local phone set support. For example, a transfer
 * attempt could fail with a PCallNotSupportedException if the transfer 
 * target is incompatible.
 * 
 * @author Robert J. Andreasen, Jr.
 */
public class PCallNotSupportedException extends PCallException
{
    /** 
     * Constructs an exception with the specified description message.
     *
     * @param description A detailed description of the exception. This 
     *        description should contain enough information to identify 
     *        exactly where or why the exception was caused.
     */
    public PCallNotSupportedException(String description)
    {
        super(description) ;
    }        


    /** 
     * Constructs an exception without a description message.
     */
    public PCallNotSupportedException()
    {
        super() ;
    }
        
}
