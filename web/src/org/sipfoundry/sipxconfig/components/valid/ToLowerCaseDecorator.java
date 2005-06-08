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
package org.sipfoundry.sipxconfig.components.valid;

import org.apache.tapestry.form.IFormComponent;

/**
 * Validation decorator that changes user input to lowercase before it gets validated
 * ToLowerCaseDecorator
 */
public class ToLowerCaseDecorator extends ValidatorDecorator {
    protected String preValidate(IFormComponent field_, String input) {
        return input.toLowerCase();
    }
}
