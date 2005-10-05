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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.valid.ValidationConstraint;

import junit.framework.TestCase;

public class SipxValidationDelegateTest extends TestCase {
    public void testGetHasSuccess() {
        SipxValidationDelegate delegate = new SipxValidationDelegate();
        assertFalse(delegate.getHasSuccess());
        delegate.recordSuccess("bongo");
        assertTrue(delegate.getHasSuccess());
        assertEquals("bongo", delegate.getSuccess());
        delegate.clear();
        assertFalse(delegate.getHasSuccess());
    }

    public void testGetHasSuccessWithErrors() {
        SipxValidationDelegate delegate = new SipxValidationDelegate();
        delegate.recordSuccess("bongo");
        delegate.record("error", ValidationConstraint.CONSISTENCY);
        assertFalse(delegate.getHasSuccess());
    }    
}
