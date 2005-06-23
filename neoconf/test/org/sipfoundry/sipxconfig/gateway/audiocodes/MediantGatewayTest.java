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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.StringWriter;
import java.io.Writer;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

import junit.framework.TestCase;

public class MediantGatewayTest extends TestCase {
    private GatewayContext m_gatewayContext;

    protected void setUp() throws Exception {
        m_gatewayContext = (GatewayContext) TestHelper.getApplicationContext().getBean(
                "gatewayContextImpl");
    }

    public void testGenerateProfiles() {
        MediantGateway gateway = new MediantGateway();
        gateway.setGatewayContext(m_gatewayContext);
        Writer writer = new StringWriter();
        gateway.generateProfiles(writer);

        System.err.println(writer.toString());
        // cursory check for now
        assertTrue(writer.toString().indexOf("SNMP") >= 0);
    }
}
