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
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.common.UserException;

public class UnknownModelException extends UserException {
    public UnknownModelException(String modelName) {
        super("Unknown model name: " + modelName);
    }
}
