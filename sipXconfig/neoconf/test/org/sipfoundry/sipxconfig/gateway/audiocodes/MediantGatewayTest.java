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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class MediantGatewayTest extends TestCase {
    private GatewayContext m_gatewayContext;
    private ModelFilesContext m_modelFilesContext;

    protected void setUp() throws Exception {
        m_gatewayContext = (GatewayContext) TestHelper.getApplicationContext().getBean(
                "gatewayContextImpl");
        m_modelFilesContext = (ModelFilesContext) TestHelper.getApplicationContext().getBean(
                "modelFilesContext");
    }

    public void testGenerateProfiles() throws Exception {
        MediantGateway gateway = new MediantGateway();
        gateway.setGatewayContext(m_gatewayContext);
        gateway.setModelFilesContext(m_modelFilesContext);
        Writer writer = new StringWriter();
        gateway.generateProfiles(writer);

        System.err.println(writer.toString());
        // cursory check for now
        assertTrue(writer.toString().indexOf("SNMP") >= 0);
    }
}
