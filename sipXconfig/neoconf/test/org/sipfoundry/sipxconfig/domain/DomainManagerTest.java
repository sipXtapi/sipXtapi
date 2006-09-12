/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import junit.framework.TestCase;

public class DomainManagerTest extends TestCase {

    public void testGetDomain() {
        DomainManager domainManager = new DomainManagerImpl();
        try {
            domainManager.getDomain();
            fail();
        } catch (RuntimeException expected) {
            assertTrue(true);
        }
    }
}
