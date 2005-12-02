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
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.common.Permission;
import org.sipfoundry.sipxconfig.gateway.Gateway;

public class AuthRulesTest extends XMLTestCase {
    private static final int GATEWAYS_LEN = 5;

    public AuthRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDoc() throws Exception {
        AuthRules rules = new AuthRules();
        Document doc = rules.getDocument();

        String xml = XmlUnitHelper.asString(doc);
        assertXMLEqual(
                "<mappings xmlns=\"http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00\"/>",
                xml);
    }

    public void testGenerate() throws Exception {
        Gateway gateway = new Gateway();
        gateway.setUniqueId();
        gateway.setAddress("10.1.2.3");

        List gateways = new ArrayList();
        gateways.add(gateway);

        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getDescription(), "test rule description");
        control.expectAndReturn(rule.getTransformedPatterns(), new String[] {
            "555", "666", "777"
        });
        control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] {
            Permission.VOICEMAIL
        }));
        control.expectAndReturn(rule.getGateways(), gateways);
        control.expectAndReturn(rule.getName(), "testrule");
        control.replay();

        MockAuthRules authRules = new MockAuthRules();
        authRules.generate(rule);
        authRules.end();

        Document document = authRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo("test rule description", "/mappings/hostMatch/description", domDoc);
        assertXpathEvaluatesTo(gateway.getAddress(), "/mappings/hostMatch/hostPattern", domDoc);
        assertXpathEvaluatesTo("555", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("666", "/mappings/hostMatch/userMatch/userPattern[2]", domDoc);
        assertXpathEvaluatesTo("777", "/mappings/hostMatch/userMatch/userPattern[3]", domDoc);
        assertXpathEvaluatesTo("Voicemail",
                "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);

        // check if generate no access has been called properly
        assertEquals(1, authRules.uniqueGateways);

        control.verify();
    }

    public void testGenerateMultipleGateways() throws Exception {
        Gateway[] gateways = new Gateway[GATEWAYS_LEN];
        for (int i = 0; i < gateways.length; i++) {
            gateways[i] = new Gateway();
            gateways[i].setUniqueId();
            gateways[i].setAddress("10.1.2." + i);
        }

        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getDescription(), null);
        control.expectAndReturn(rule.getTransformedPatterns(), new String[] {
            "555", "666", "777"
        });
        control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] {
            Permission.VOICEMAIL
        }));
        control.expectAndReturn(rule.getGateways(), Arrays.asList(gateways));
        control.expectAndReturn(rule.getName(), "testrule");
        control.replay();

        AuthRules authRules = new AuthRules();
        authRules.generate(rule);
        authRules.end();

        Document document = authRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        String hostMatch = "/mappings/hostMatch/";
        for (int i = 0; i < gateways.length; i++) {
            String hostPattern = "hostPattern[" + (i + 1) + "]";
            assertXpathEvaluatesTo(gateways[i].getAddress(), hostMatch + hostPattern, domDoc);
        }
        assertXpathEvaluatesTo("555", hostMatch + "userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("666", hostMatch + "userMatch/userPattern[2]", domDoc);
        assertXpathEvaluatesTo("777", hostMatch + "userMatch/userPattern[3]", domDoc);
        assertXpathEvaluatesTo("Voicemail", hostMatch + "/userMatch/permissionMatch/permission",
                domDoc);

        String lastHostMatch = "/mappings/hostMatch[2]/";
        // "no access" match at the end of the file - just checks if paths are that
        // testGenerateNoAccessRule tests if values are correct
        for (int i = 0; i < gateways.length; i++) {
            assertXpathExists(lastHostMatch + "hostPattern[" + (i + 1) + "]", domDoc);
        }

        assertXpathEvaluatesTo(".", lastHostMatch + "userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("NoAccess",
                lastHostMatch + "userMatch/permissionMatch/permission", domDoc);

        control.verify();
    }

    public void testGenerateNoPermissionRequiredRule() throws Exception {
        Gateway[] gateways = new Gateway[GATEWAYS_LEN];
        for (int i = 0; i < gateways.length; i++) {
            gateways[i] = new Gateway();
            gateways[i].setUniqueId();
            gateways[i].setAddress("10.1.2." + i);
        }

        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getDescription(), null);
        control.expectAndReturn(rule.getTransformedPatterns(), new String[] {
            "555", "666", "777"
        });
        control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] {}));
        control.expectAndReturn(rule.getGateways(), Arrays.asList(gateways));
        control.expectAndReturn(rule.getName(), "testrule");
        control.replay();

        MockAuthRules authRules = new MockAuthRules();
        authRules.generate(rule);
        authRules.end();

        Document document = authRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        String hostMatch = "/mappings/hostMatch/";
        for (int i = 0; i < gateways.length; i++) {
            String hostPattern = "hostPattern[" + (i + 1) + "]";
            assertXpathEvaluatesTo(gateways[i].getAddress(), hostMatch + hostPattern, domDoc);
        }
        assertXpathEvaluatesTo("555", hostMatch + "userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("666", hostMatch + "userMatch/userPattern[2]", domDoc);
        assertXpathEvaluatesTo("777", hostMatch + "userMatch/userPattern[3]", domDoc);
        assertXpathEvaluatesTo("", hostMatch + "/userMatch/permissionMatch", domDoc);

        // check if generate no access has been called properly
        assertEquals(GATEWAYS_LEN, authRules.uniqueGateways);

        control.verify();
    }

    public void testGenerateNoAccessRule() throws Exception {
        Gateway[] gateways = new Gateway[GATEWAYS_LEN];
        for (int i = 0; i < gateways.length; i++) {
            gateways[i] = new Gateway();
            gateways[i].setUniqueId();
            gateways[i].setAddress("10.1.2." + i);
        }
        AuthRules rules = new AuthRules();
        rules.generateNoAccess(Arrays.asList(gateways));
        String lastHostMatch = "/mappings/hostMatch/";
        Document document = rules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);
        // "no access" match at the end of the file
        for (int i = 0; i < gateways.length; i++) {
            assertXpathEvaluatesTo(gateways[i].getAddress(), lastHostMatch + "hostPattern["
                    + (i + 1) + "]", domDoc);
        }

        assertXpathEvaluatesTo(".", lastHostMatch + "userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("NoAccess",
                lastHostMatch + "userMatch/permissionMatch/permission", domDoc);
    }

    public void testNamespace() {
        AuthRules rules = new AuthRules();
        Document doc = rules.getDocument();

        Element rootElement = doc.getRootElement();
        XmlUnitHelper.assertElementInNamespace(rootElement,
                "http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00");
    }

    private class MockAuthRules extends AuthRules {
        public int uniqueGateways = 0;

        void generateNoAccess(List gateways) {
            uniqueGateways = gateways.size();
        }
    }
}
