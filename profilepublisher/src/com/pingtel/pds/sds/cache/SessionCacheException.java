/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.sds.cache;

import com.pingtel.pds.common.PDSException;

/**
 * place holder Cache exceptions
 */
public class SessionCacheException extends PDSException {
    /**
     * Constructs a generic <code>CommandException</code> with the specified
     * detail message.
     *
     * @param   message   the detail message.
     */
    public SessionCacheException ( String message ) {
        super ( message );
    }

    /**
     * Constructs a generic <code>CommandException</code> with the specified
     * detail message and a wrapped exception.
     *
     * @param   message   the detail message.
     */
    public SessionCacheException ( String message, Exception ex ) {
        super ( message, ex );
    }
}