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

package com.pingtel.pds.sds.cmd;

import com.pingtel.pds.common.PDSException;

public class CommandException extends PDSException {
    /**
     * Constructs a generic <code>CommandException</code> with the specified
     * detail message.
     *
     * @param   message   the detail message.
     */
    public CommandException ( String message ) {
        super ( message );
    }

    /**
     * Constructs a generic <code>CommandException</code> with the specified
     * detail message.
     *
     * @param   message   the detail message.
     */
    public CommandException ( String message, Exception ex ) {
        super ( message, ex );
    }
}