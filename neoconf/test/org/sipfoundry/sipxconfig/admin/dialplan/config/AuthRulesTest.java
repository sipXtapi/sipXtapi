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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.Element;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class AuthRulesTest extends XMLTestCase {
    public AuthRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }
    
    public void testGetDoc() throws Exception {
        AuthRules rules = new AuthRules();
        Document doc = rules.getDocument();

        String xml = XmlUnitHelper.asString(doc);
        assertXMLEqual("<mappings/>", xml);
    }

    public void testGenerate() throws Exception {
        Gateway gateway = new Gateway();
        gateway.setAddress("10.1.2.3");

        List gateways = new ArrayList();
        gateways.add(gateway);

        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getPatterns(), new String[] { "555", "666", "777" });
        control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] { Permission.VOICEMAIL }));
        control.expectAndReturn(rule.getGateways(), gateways);
        control.expectAndReturn(rule.getName(), "testrule");
        control.replay();

        AuthRules authRules = new AuthRules();
        authRules.generate(rule);

        Document document = authRules.getDocument();        
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo(gateway.getAddress(), "/mappings/hostMatch/hostPattern", domDoc);
        assertXpathEvaluatesTo("555", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("666", "/mappings/hostMatch/userMatch/userPattern[2]", domDoc);
        assertXpathEvaluatesTo("777", "/mappings/hostMatch/userMatch/userPattern[3]", domDoc);
        assertXpathEvaluatesTo("Voicemail",
                "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);

        control.verify();
    }
    
    public void testNamespace() {
        AuthRules rules = new AuthRules();
        Document doc = rules.getDocument();
        
        Element rootElement = doc.getRootElement();
        XmlUnitHelper.assertElementInNamespace(rootElement, "http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00");        
    }
}
