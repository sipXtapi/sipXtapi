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

package com.pingtel.pds.sds.sip;

import com.pingtel.pds.common.PDSException;

/**
 * <p>Title: </p>
 * <p>Description: a SipException handler class</p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: </p>
 * @author unascribed
 * @version 1.0
 */
public class SipException extends PDSException {
    /**
     * Constructs a generic <code>SipException</code> with the specified
     * detail message.
     *
     * @param   message   the detail message.
     */
    public SipException ( String message ) {
        super ( message );
    }

    /**
     * Constructs a generic <code>SipException</code> with the specified
     * detail message.
     *
     * @param   message   the detail message.
     */
    public SipException ( String message, Exception ex ) {
        super ( message, ex );
    }
}
