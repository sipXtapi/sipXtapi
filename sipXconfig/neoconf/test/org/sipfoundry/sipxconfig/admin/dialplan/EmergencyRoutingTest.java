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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.List;

import org.sipfoundry.sipxconfig.gateway.Gateway;

import junit.framework.TestCase;

public class EmergencyRoutingTest extends TestCase {
    private static final String DEFAULT_EXTENSION = "333";
    private final static String TEST_DATA[][] = {
        {
            "911", "default.abc.com"
        }, {
            "920", "g1.abc.com"
        }, {
            "911", "g2.abc.com"
        }
    };
    private Gateway m_defaultGateway;

    protected void setUp() throws Exception {
        m_defaultGateway = new Gateway();
        m_defaultGateway.setAddress("aaa.bbb.com");
    }

    public void testAsDialingRulesListEmpty() {
        EmergencyRouting routing = new EmergencyRouting();
        List rules = routing.asDialingRulesList();
        assertTrue(rules.isEmpty());
    }

    public void testAsDialingRulesListNoExceptions() {
        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(m_defaultGateway);
        routing.setExternalNumber(DEFAULT_EXTENSION);
        List rules = routing.asDialingRulesList();
        assertEquals(1, rules.size());

        IDialingRule rule = (IDialingRule) rules.get(0);
        List gateways = rule.getGateways();
        assertEquals(1, gateways.size());
        assertEquals(m_defaultGateway.getAddress(), ((Gateway) gateways.get(0)).getAddress());

        assertTrue(rule.getPermissions().isEmpty());

        assertEquals(1, rule.getPatterns().length);

        assertEquals(DEFAULT_EXTENSION, rule.getPatterns()[0]);
    }

    public void testAsDialingRulesListFull() {
        EmergencyRouting routing = new EmergencyRouting();
        routing.setDefaultGateway(m_defaultGateway);
        routing.setExternalNumber(DEFAULT_EXTENSION);
        for (int i = 0; i < TEST_DATA.length; i++) {
            Gateway gateway = new Gateway();
            gateway.setAddress(TEST_DATA[i][1]);

            RoutingException re = new RoutingException("abc, ddd", TEST_DATA[i][0], gateway);
            routing.addException(re);
        }

        List rules = routing.asDialingRulesList();
        assertEquals(TEST_DATA.length + 1, rules.size());

        for (int i = 1; i < TEST_DATA.length; i++) {
            IDialingRule rule = (IDialingRule) rules.get(i + 1);
            List gateways = rule.getGateways();
            assertEquals(1, gateways.size());
            Gateway g = (Gateway) gateways.get(0);

            assertEquals(TEST_DATA[i][1], g.getAddress());

            assertTrue(rule.getPermissions().isEmpty());

            assertEquals(1, rule.getPatterns().length);

            assertEquals(TEST_DATA[i][0], rule.getPatterns()[0]);
        }
    }
}
