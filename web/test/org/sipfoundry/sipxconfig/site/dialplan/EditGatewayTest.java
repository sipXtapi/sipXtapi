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

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;

import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * EditGatewayTest
 */
public class EditGatewayTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();
    private EditGateway m_editGatewayPage;
    private DialPlanManager m_manager;

    protected void setUp() throws Exception {
        m_editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
        // Tapestry takes care of creating the manager in run-time
        m_manager = new DialPlanManager();
        m_editGatewayPage.setDialPlanManager(m_manager);
    }

    public void testAddNewGateway() {
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();

        // return to list gateways page in normal case
        cycle.activate(ListGateways.PAGE);
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);
        cycleControl.verify();

        assertEquals(1, m_manager.getGateways().size());
        Object addedGateway = m_manager.getGateways().get(0);
        Gateway gateway = m_editGatewayPage.getGateway();
        assertEquals(gateway, addedGateway);
    }

    public void testSaveAndAssign() {
        // TODO: these should be mocked
        IDialingRule rule = new CustomDialingRule();
        rule.setName("my rule name");
        FlexibleDialPlan flexDialPlan = m_manager.getFlexDialPlan();
        flexDialPlan.addRule(rule);
        Gateway gateway = new Gateway();
        gateway.setName("testName");

        m_editGatewayPage.setDialPlanManager(m_manager);
        m_editGatewayPage.setGateway(gateway);
        m_editGatewayPage.setRuleId(rule.getId());

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate(EditCustomDialRule.PAGE);
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);
        cycleControl.verify();

        assertEquals(1, m_manager.getGateways().size());
        Object addedGateway = m_manager.getGateways().get(0);
        assertEquals(gateway, addedGateway);

        assertEquals(1, rule.getGateways().size());
        assertTrue(rule.getGateways().contains(gateway));
    }

    public void testPageBeginRenderAdd() {
        m_editGatewayPage.pageBeginRender(null);

        assertNotNull(m_editGatewayPage.getGateway());
        assertNull(m_editGatewayPage.getGatewayId());
    }

    public void testPageBeginRenderEdit() {
        DialPlanManager manager = new DialPlanManager();
        Gateway gateway = new Gateway();
        gateway.setName("kuku");
        Integer id = gateway.getId();
        manager.addGateway(gateway);

        m_editGatewayPage.setDialPlanManager(manager);
        m_editGatewayPage.setGatewayId(id);
        m_editGatewayPage.pageBeginRender(null);

        assertEquals(id, m_editGatewayPage.getGatewayId());
        assertEquals("kuku", m_editGatewayPage.getGateway().getName());
        assertFalse(gateway.equals(m_editGatewayPage.getGateway()));

    }
}
