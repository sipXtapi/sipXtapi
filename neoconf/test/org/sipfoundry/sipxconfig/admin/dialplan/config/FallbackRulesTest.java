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

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * MappingRulesTest
 */
public class FallbackRulesTest extends XMLTestCase {

    public void testGenerateRuleWithGateways() throws Exception {
        Gateway g1 = new Gateway();
        g1.setAddress("10.1.1.14");
        List gateways = Arrays.asList(new Gateway[] { g1 });
        FullTransform t1 = new FullTransform();
        t1.setUser("333");
        t1.setHost(g1.getAddress());
        t1.setFieldParams(new String[] {"Q=0.97"});

        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getPatterns(), new String[] { "x." });
        //control.expectAndReturn(rule.getPermissions(), Arrays.asList(new Permission[] { Permission.VOICEMAIL }));
        control.expectAndReturn(rule.getGateways(), gateways, 2);
        control.expectAndReturn(rule.getTransforms(), new Transform[] { t1 });
        control.replay();

        MappingRules mappingRules = new FallbackRules();
        mappingRules.generate(rule);

        Document document = mappingRules.getDocument();
        org.w3c.dom.Document domDoc = TransformTest.getDomDoc(document);

        assertXpathEvaluatesTo("x.", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);
        assertXpathEvaluatesTo("333",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/user", domDoc);
        assertXpathEvaluatesTo(g1.getAddress(),
                "/mappings/hostMatch/userMatch/permissionMatch/transform/host", domDoc);
        assertXpathEvaluatesTo("Q=0.97",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/fieldparams", domDoc);

        control.verify();
    }

    public void testGenerateRuleWithoutGateways() throws Exception {
        MockControl control = MockControl.createControl(IDialingRule.class);
        IDialingRule rule = (IDialingRule) control.getMock();
        control.expectAndReturn(rule.getGateways(), Collections.EMPTY_LIST);
        control.replay();

        MappingRules mappingRules = new FallbackRules();
        mappingRules.generate(rule);

        Document document = mappingRules.getDocument();
        org.w3c.dom.Document domDoc = TransformTest.getDomDoc(document);

        assertXpathNotExists("/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch", domDoc);
        assertXpathExists("/mappings/hostMatch/hostPattern", domDoc);

        control.verify();
    }
}
