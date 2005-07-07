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

import junit.framework.TestCase;

/**
 * SIPxchangeLoginModuleTest
 */
public class SIPxchangeLoginModuleTest extends TestCase {

    public void testNormalizeUserName() {

        String username = SIPxchangeLoginModule.normalizeUserName("sip:user");
        assertEquals("user", username);
        username = SIPxchangeLoginModule.normalizeUserName("sip:user@kuku");
        assertEquals("user", username);
        username = SIPxchangeLoginModule.normalizeUserName("user");
        assertEquals("user", username);
    }

    public void testCreate() {
        try {
            new SIPxchangeLoginModule();
            // this is the list of classes without which JBoss will not be able
            // to create
            // authentication module
            Class.forName("com.pingtel.pds.common.VersionInfo");
            Class.forName("com.pingtel.pds.common.PathLocatorUtil");
            Class.forName("com.pingtel.pds.common.MD5Encoder");
            Class.forName("com.pingtel.pds.common.PathBuffer");
        } catch (Exception e) {
            fail("Cannot create authorization module for JBoss" + e);
        }
    }

}
