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

public class NameInUseException extends UserException {
    private static final String ERROR = "There is already a {0} with the name \"{1}\". "
            + "Please choose another name for this {0}.";

    public NameInUseException(String objectType, String name) {
        super(ERROR, objectType, name);
    }
}
