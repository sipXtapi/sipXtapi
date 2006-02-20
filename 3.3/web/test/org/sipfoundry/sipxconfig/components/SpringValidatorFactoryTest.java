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

import junit.framework.TestCase;

public class SpringValidatorFactoryTest extends TestCase {

    /*
     * Test method for 'org.sipfoundry.sipxconfig.components.SpringValidatorFactory.splitSpecification(String)'
     */
    public void testSplitSpecification() {
        SpringValidatorFactory factory = new SpringValidatorFactory();     
        assertArrayEquals(new String[] {""}, factory.splitSpecification(""));
        assertArrayEquals(new String[] {"a", "b"}, factory.splitSpecification("a, b"));
        assertArrayEquals(new String[] {"a", "b=x", "c=y"}, factory.splitSpecification("a, b=x, c=y"));
    }
    
    void assertArrayEquals(String[] expected, String[] actual) {
        assertEquals(expected.length, actual.length);
        for (int i = 0; i < expected.length; i++) {
            assertEquals(expected[i], actual[i]);
        }
    }
}
