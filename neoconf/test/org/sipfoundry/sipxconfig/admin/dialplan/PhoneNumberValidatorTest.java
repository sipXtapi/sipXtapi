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

import junit.framework.TestCase;

/**
 * PhoneNumberValidatorTest
 */
public class PhoneNumberValidatorTest extends TestCase {

    public void testValidateString() {
        PhoneNumberValidator validator = new PhoneNumberValidator();
        assertTrue(validator.validateString(""));
        assertTrue(validator.validateString("234245"));
        assertFalse(validator.validateString("234d245"));
    }
}
