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
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.StringUtils;

import junit.framework.TestCase;

/**
 * RoutingExceptionTest
 */
public class RoutingExceptionTest extends TestCase {
    public void testGetCallers() {
        RoutingException exception = new RoutingException("123, 455", "922", null);
        String[] patterns = exception.getPatterns("domain.com");
        assertEquals(2, patterns.length);
        assertEquals("123@domain.com", patterns[0]);
        assertEquals("455@domain.com", patterns[1]);        
    }

    public void testGetCallersEmpty() {
        RoutingException exception = new RoutingException();
        String[] patterns = exception.getPatterns(StringUtils.EMPTY);
        assertEquals(0, patterns.length);
    }
}
