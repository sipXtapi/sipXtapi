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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;

/**
 * EmergencyRoutingRulesTest
 */
public class EmergencyRoutingRulesTest extends XMLTestCase {
    private EmergencyRouting m_routing;

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        
        Gateway g1 = new Gateway();
        g1.setAddress("47.1.2.22");
        RoutingException e1 = new RoutingException("823124234, 723124234, 422344234", "9911", g1);
        
        Gateway g2 = new Gateway();
        g2.setAddress("10.1.2.1");
        RoutingException e2 = new RoutingException("623124234, 723125344, 8354234, 422344234", "911", g2);
        
        Gateway g3 = new Gateway();
        g3.setAddress("xxx.yyy.com");
        RoutingException e3 = new RoutingException("523124234", "922", g3);
        
        m_routing = new EmergencyRouting();
        m_routing.addException(e1);
        m_routing.addException(e2);
        m_routing.addException(e3);

        Gateway g4 = new Gateway();
        g4.setAddress("10.2.3.4");        
        m_routing.setDefaultGateway(g4);
        m_routing.setEnabled(true);
        m_routing.setExternalNumber("919");
    }

    public void testGenerate() throws Exception {
        EmergencyRoutingRules rules = new EmergencyRoutingRules();
        rules.generate(m_routing, "domain.com");
        String generatedXml = rules.getFileContent();        
        InputStream referenceXml = EmergencyRoutingRulesTest.class.getResourceAsStream("e911rules.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXml),  new StringReader(generatedXml));
    }
}
