/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.common.UserException;

public class LicenseException extends UserException {
    public static final String VIOLATION = "Cannot add {0}. Current licence limit is: {1}.";

    public LicenseException(String acdObjectName, int limit) {
        super(VIOLATION, acdObjectName, Integer.toString(limit));
    }
}
