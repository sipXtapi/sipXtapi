/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

/**
 * Caught in application layer, this informs the user they've done something wrong. Despite being
 * an unchecked exception, this is not meant to be a fatal error and application layer should
 * handle it gracefully.
 */
public class UserException extends RuntimeException {

    public UserException(String message) {
        super(message);
    }

    public UserException() {
    }
}
