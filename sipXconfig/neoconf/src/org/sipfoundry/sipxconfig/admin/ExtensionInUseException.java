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
package org.sipfoundry.sipxconfig.admin;

import org.sipfoundry.sipxconfig.common.UserException;

public class ExtensionInUseException extends UserException {
    private static final String ERROR = "Extension {1} is already in use. "
            + "Please choose another extension for this {0}.";

    public ExtensionInUseException(String objectType, String extension) {
        super(ERROR, objectType, extension);
    }
}
