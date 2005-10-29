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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;

import junit.framework.TestCase;

public class SipxReplicationManagerImplTest extends TestCase {
    SipxReplicationManagerImpl m_processContextImpl;

    protected void setUp() throws Exception {
        m_processContextImpl = new SipxReplicationManagerImpl() {
            protected InputStream getTopologyAsStream() {
                return SipxReplicationManagerImplTest.class
                        .getResourceAsStream("topology.test.xml");
            }
        };
    }

    public void testRetrieveReplicationUrls() throws Exception {
        String[] urls = m_processContextImpl.getReplicationUrls();
        assertEquals(2, urls.length);
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", urls[0]);
        assertEquals("https://192.168.0.27:8091/cgi-bin/replication/replication.cgi", urls[1]);
    }
}
