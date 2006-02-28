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
package org.sipfoundry.sipxconfig.components;

import java.util.Locale;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;

/**
 * Comments
 */
public class TapestryUtilsTest extends TestCase {

    public void testAssertParameterArrayBounds() {
        try {
            TapestryUtils.assertParameter(String.class, null, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testAssertParameterWrongClass() {
        Object[] strings = new Object[] {
            "i'm a string"
        };
        try {
            TapestryUtils.assertParameter(Integer.class, strings, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testAssertParameterIsAssignableFrom() {
        Object[] strings = new Object[] {
            "i'm a string"
        };
        TapestryUtils.assertParameter(Object.class, strings, 0);
        assertTrue(true);
    }

    public void testAssertParameterNotAssignableFrom() {
        Object[] objects = new Object[] {
            new Object()
        };
        try {
            TapestryUtils.assertParameter(String.class, objects, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testGetMessages() throws Exception {
        Messages messages = new DummyMessages();

        assertEquals("cde", TapestryUtils.getMessage(messages, "abc", "cde"));
        assertNull("cde", TapestryUtils.getMessage(messages, "xyz", null));
        assertEquals("dummy label", TapestryUtils.getMessage(messages, "dummy", null));
    }

    private class DummyMessages extends AbstractMessages {
        protected Locale getLocale() {
            return Locale.US;
        }

        protected String findMessage(String key) {
            if (key.equals("dummy")) {
                return "dummy label";
            }
            return null;
        }
    }
}
