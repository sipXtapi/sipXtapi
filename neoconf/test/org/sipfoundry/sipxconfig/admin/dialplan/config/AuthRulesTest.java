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

import java.io.StringWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.MockControl;

import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class AuthRulesTest extends XMLTestCase {

    public void testGetDoc() throws Exception {
        AuthRules rules = new AuthRules();
        Document doc = rules.getDocument();

        StringWriter xml = new StringWriter();
        doc.write(xml);
        assertXMLEqual("<mappings/>", xml.toString());
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
        org.w3c.dom.Document domDoc = TransformTest.getDomDoc(document);

        TransformTest.dumpXml(document);

        assertXpathEvaluatesTo(gateway.getAddress(), "/mappings/hostMatch/hostPattern", domDoc);
        assertXpathEvaluatesTo("555", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("666", "/mappings/hostMatch/userMatch/userPattern[2]", domDoc);
        assertXpathEvaluatesTo("777", "/mappings/hostMatch/userMatch/userPattern[3]", domDoc);
        assertXpathEvaluatesTo("Voicemail",
                "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);

        control.verify();
    }
}
