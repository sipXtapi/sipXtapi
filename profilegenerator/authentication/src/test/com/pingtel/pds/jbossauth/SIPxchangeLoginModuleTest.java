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
package com.pingtel.pds.jbossauth;

import com.pingtel.pds.jbossauth.SIPxchangeLoginModule;

import junit.framework.TestCase;

/**
 * SIPxchangeLoginModuleTest
 */
public class SIPxchangeLoginModuleTest extends TestCase {

    public void testNormalizeUserName() {
        
        SIPxchangeLoginModule loginModule = new SIPxchangeLoginModule();
        String username = loginModule.normalizeUserName("sip:user");
        assertEquals("user",username);
        username = loginModule.normalizeUserName("sip:user@kuku");
        assertEquals("user",username);
        username = loginModule.normalizeUserName("user");
        assertEquals("user",username);        
    }

}
