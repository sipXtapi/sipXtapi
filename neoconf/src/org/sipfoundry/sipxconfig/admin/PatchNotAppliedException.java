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
package org.sipfoundry.sipxconfig.admin;

/**
 * A patch wasn't applied that needs to be. 
 */
public class PatchNotAppliedException extends RuntimeException {
    
    public PatchNotAppliedException(String message) {
        super(message);
    }
}
