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
package org.sipfoundry.sipxconfig.phone;


/**
 * System was unable to restart phone  
 */
public class RestartException extends RuntimeException {
    
    public RestartException(String msg) {
        super(msg);
    }
    
    public RestartException(String msg, Throwable cause) {
        super(msg, cause);
    }
}
