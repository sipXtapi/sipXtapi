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
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.VisitorSupport;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.common.Permission;

/**
 * MappingRulesTest
 */
public class MappingRulesTest extends XMLTestCase {
    public MappingRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDocument() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${SIPXCHANGE_DOMAIN_NAME}", "/mappings/hostMatch/hostPattern",
                xml);
        assertXpathEvaluatesTo("${MY_FULL_HOSTNAME}", "/mappings/hostMatch/hostPattern[2]", xml);
        assertXpathEvaluatesTo("${MY_HOSTNAME}", "/mappings/hostMatch/hostPattern[3]", xml);
        assertXpathEvaluatesTo("${MY_IP_ADDR}", "/mappings/hostMatch/hostPattern[4]", xml);
    }

    public void testGetDocumentInvalidExternals() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.setExternalRulesFileName("/invalid/file/name");
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
    }

    public void testGetDocumentValidExternals() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        MappingRules mappingRules = new MappingRules();
        mappingRules.setExternalRulesFileName(resource.getFile());
        mappingRules.begin();
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathEvaluatesTo("some_other.domain.com", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${SIPXCHANGE_DOMAIN_NAME}",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/host", xml);

        assertXpathExists("/mappings/hostMatch[2]/hostPattern", xml);
    }

    /**
     * This is mostly to demonstrate how complicated the XPatch expression becomes for a document
     * with a namespace
     * 
     * @param document
     */
    static void dumpXPaths(Document document) {
        VisitorSupport support = new VisitorSupport() {
            public void visit(Element node) {
                System.err.println(node.getPath());
            }
        };
        document.accept(support);
    }

    public void testGetHostMatch() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        Element hostMatch = mappingRules.getFirstHostMatch();
        Document document = mappingRules.getDocument();
        assertSame(document, hostMatch.getDocument());
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");
        assertEquals("/*[name()='mappings']/*[name()='hostMatch']", hostMatch.getPath());
    }

    public void testGenerate() throws Exception {
        UrlTransform voicemail = new UrlTransform();
        voicemail
                .setUrl("<sip:{digits}@{mediaserver};"
                        + "play={voicemail}/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.1");

        UrlTransform voicemail2 = new UrlTransform();
        voicemail2
                .setUrl("<sip:{digits}@testserver;"
                        + "play={voicemail}/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.001");

        MockControl control = MockControl.createStrictControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.isInternal(), true);
        control.expectAndReturn(rule.getName(), null);
        control.expectAndReturn(rule.getDescription(), "my rule description");
        control.expectAndReturn(rule.getPatterns(), new String[] {
            "x."
        });
        control.expectAndReturn(rule.isInternal(), true);
        control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] {
            Permission.VOICEMAIL
        }));
        control.expectAndReturn(rule.getTransforms(), new Transform[] {
            voicemail, voicemail2
        });
        control.replay();

        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();

        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo("my rule description",
                "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("x.", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("Voicemail",
                "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);
        assertXpathEvaluatesTo(voicemail.getUrl(),
                "/mappings/hostMatch/userMatch/permissionMatch/transform/url", domDoc);
        assertXpathEvaluatesTo(voicemail2.getUrl(),
                "/mappings/hostMatch/userMatch/permissionMatch/transform[2]/url", domDoc);

        control.verify();
    }

    public void testGenerateRuleWithGateways() throws Exception {
        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.isInternal(), false);
        control.replay();

        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathNotExists("/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch", domDoc);
        assertXpathExists("/mappings/hostMatch/hostPattern", domDoc);

        control.verify();
    }

    public void testInternalRules() throws Exception {
        int extension = 3;
        List rules = new ArrayList();
        AutoAttendant aa = AutoAttendant.createOperator();
        rules.add(new MappingRule.Operator(aa, "100", new String[] {
            "operator", "0"
        }));
        rules.add(new MappingRule.Voicemail("101"));
        rules.add(new MappingRule.VoicemailTransfer("2", extension));
        rules.add(new MappingRule.VoicemailFallback(3));

        MockControl controlPlan = MockControl.createStrictControl(DialPlanContext.class);
        DialPlanContext plan = (DialPlanContext) controlPlan.getMock();
        plan.getGenerationRules();
        controlPlan.setReturnValue(rules);
        plan.getAttendantRules();
        controlPlan.setReturnValue(Collections.EMPTY_LIST);
        controlPlan.replay();

        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(plan, null);

        String generatedXml = generator.getFileContent(ConfigFileType.MAPPING_RULES);

        InputStream referenceXmlStream = MappingRulesTest.class
                .getResourceAsStream("mappingrules.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        controlPlan.verify();
    }

    public void testEquals() throws Exception {
        XmlFile f1 = new MappingRules();
        XmlFile f2 = new MappingRules();
        XmlFile f3 = new FallbackRules();
        assertEquals(f1, f2);
        assertNotSame(f1, f2);
        assertEquals(f1.hashCode(), f2.hashCode());
        assertFalse(f1.equals(null));
        assertFalse(f1.equals(f3));
    }
}
