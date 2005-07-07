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

public class SipxProcessContextImplTest extends TestCase {

    public void testConstructRestartUrl() {
        SipxProcessContextImpl processContextImpl = new SipxProcessContextImpl() {
            protected String retrieveProcessUrl() {
                return "https://localhost:8091/cgi-bin/processmonitor/process.cgi";
            }            
        };
        String url = processContextImpl
                .constructRestartUrl(SipxProcessContext.Process.REGISTRAR);
        
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=SIPRegistrar",
                url);
    }
    
    
    public void testRetrieveProcesUrl() throws Exception {
        SipxProcessContextImpl processContextImpl = new SipxProcessContextImpl();
        InputStream stream = SipxProcessContextImplTest.class.getResourceAsStream("topology.test.xml");
        String url = processContextImpl.retrieveProcessUrl(stream);
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi",
                url);
    }
}
