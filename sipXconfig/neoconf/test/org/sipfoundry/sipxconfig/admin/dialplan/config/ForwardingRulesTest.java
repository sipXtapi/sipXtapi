/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.HostPatternProvider;

public class ForwardingRulesTest extends XMLTestCase {

    public void testGenerate() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        HostPatternProvider rule = control.createMock(HostPatternProvider.class);
        rule.getHostPatterns();
        control.andReturn(new String[] { "gander" });
        
        control.replay();

        ForwardingRules rules = new ForwardingRules();
        rules.setVelocityEngine(TestHelper.getVelocityEngine());
        rules.begin();
        rules.generate(rule);
        rules.end();

        Document document = rules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo("gander", "/routes/route/routeFrom[5]", domDoc);

        control.verify();
    }
}
