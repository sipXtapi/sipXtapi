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

import junit.framework.TestCase;

import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

/**
 * SelectGatewaysTest
 */
public class SelectGatewaysTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();
    private SelectGateways m_page;

    protected void setUp() throws Exception {
        m_page = (SelectGateways) m_pageMaker.getInstance(SelectGateways.class);
    }

    public void testSelectGateways() {
        List gatewaysToAdd = new ArrayList();
        List gateways = new ArrayList(); 
        for(int i = 0; i< 3; i++) {
            Gateway gateway = new Gateway();
            gateway.setUniqueId();
            gatewaysToAdd.add(gateway.getId());
            gateways.add(gateway);
        }
        
        DialingRule rule = new CustomDialingRule();
        
        MockControl controlPlan = MockControl.createStrictControl(FlexibleDialPlanContext.class);
        controlPlan.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        FlexibleDialPlanContext flexDialPlan = (FlexibleDialPlanContext) controlPlan.getMock();
        flexDialPlan.getRule(rule.getId());
        controlPlan.setReturnValue(rule);
        flexDialPlan.storeRule(rule);
        controlPlan.replay();
        
        
        MockControl control = MockControl.createControl(DialPlanContext.class);
        DialPlanContext context = (DialPlanContext) control.getMock();
        
        control.expectAndReturn(context.getGatewayByIds(gatewaysToAdd), gateways);
        control.expectAndReturn(context.getFlexDialPlan(), flexDialPlan);
                
        control.replay();

        m_page.setDialPlanManager(context);
        m_page.setRuleId(rule.getId());
        m_page.selectGateways(gatewaysToAdd);
        
        List ruleGateways = rule.getGateways();
        assertEquals(gatewaysToAdd.size(), ruleGateways.size());
        for (Iterator i = ruleGateways.iterator(); i.hasNext();) {
            Gateway g = (Gateway) i.next();
            assertTrue(gatewaysToAdd.contains(g.getId()));
        }
        
        control.verify();
        controlPlan.verify();
    }
}
