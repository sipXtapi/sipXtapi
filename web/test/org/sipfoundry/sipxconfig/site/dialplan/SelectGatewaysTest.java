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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.test.AbstractInstantiator;

import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

import junit.framework.TestCase;

/**
 * SelectGatewaysTest
 */
public class SelectGatewaysTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();
    private SelectGateways m_page;
    private DialPlanManager m_manager;

    protected void setUp() throws Exception {
        m_manager = new DialPlanManager();
        m_page = (SelectGateways) m_pageMaker.getInstance(SelectGateways.class);
        for (int i = 0; i < 10; i++) {
            m_manager.addGateway(new Gateway());
        }
        for (int i = 0; i < 10; i++) {
            m_manager.getFlexDialPlan().addRule(new CustomDialingRule());
        }
    }

    public void testSelectGateways() {
        List gatewaysToAdd = new ArrayList();
        for(int i = 0; i< 3; i++) {
            Gateway gateway = (Gateway) m_manager.getGateways().get(i*2);
            gatewaysToAdd.add(gateway.getId());
        }
        DialingRule rule = (DialingRule) m_manager.getFlexDialPlan().getRules().get(3);
        m_page.setDialPlanManager(m_manager);
        m_page.setRuleId(rule.getId());
        m_page.selectGateways(gatewaysToAdd);
        
        List gateways = rule.getGateways();
        assertEquals(gatewaysToAdd.size(), gateways.size());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway g = (Gateway) i.next();
            assertTrue(gatewaysToAdd.contains(g.getId()));
        }
    }

}
