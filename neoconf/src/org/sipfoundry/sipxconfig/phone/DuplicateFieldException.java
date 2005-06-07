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
 * Thrown when trying to add or update objects with fields that must be unique across
 * the database and exiting values already exist.
 * 
 * @see PhoneContextImpl.checkForDuplicateFields
 */
public class DuplicateFieldException extends RuntimeException {

    public DuplicateFieldException(String message) {
        super(message);
    }
}
