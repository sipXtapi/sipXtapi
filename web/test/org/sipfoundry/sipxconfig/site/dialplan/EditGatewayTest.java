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
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * EditGatewayTest
 */
public class EditGatewayTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();
    private EditGateway m_editGatewayPage;

    protected void setUp() throws Exception {
        m_editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
    }

    public void testAddNewGateway() {
        Gateway g = new Gateway(new Integer(5));

        MockControl contextControl = MockControl.createStrictControl(DialPlanContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        DialPlanContext context = (DialPlanContext) contextControl.getMock();

        context.addGateway(g);

        contextControl.replay();
        m_editGatewayPage.setDialPlanManager(context);
        m_editGatewayPage.setGateway(g);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        m_editGatewayPage.setNextPage(ListGateways.PAGE);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate(ListGateways.PAGE);
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);

        cycleControl.verify();
        contextControl.verify();
    }

    public void testSaveAndAssign() {
        FlexibleDialPlan flexDialPlan = new FlexibleDialPlan();
        IDialingRule rule = new CustomDialingRule();
        flexDialPlan.addRule(rule);
        rule.setName("my rule name");
        Gateway g = new Gateway(new Integer(5));

        MockControl contextControl = MockControl.createStrictControl(DialPlanContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        DialPlanContext context = (DialPlanContext) contextControl.getMock();
        context.addGateway(g);
        contextControl.expectAndReturn(context.getFlexDialPlan(), flexDialPlan);
        contextControl.replay();

        m_editGatewayPage.setDialPlanManager(context);
        m_editGatewayPage.setGateway(g);
        m_editGatewayPage.setRuleId(rule.getId());
        m_editGatewayPage.setNextPage("EditCustomDialRule");
        m_editGatewayPage.pageBeginRender(null);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("EditCustomDialRule");
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);

        cycleControl.verify();
        contextControl.verify();

        assertEquals(1, rule.getGateways().size());
        assertTrue(rule.getGateways().contains(g));
    }

    public void testPageBeginRenderAdd() {
        m_editGatewayPage.pageBeginRender(null);

        assertNotNull(m_editGatewayPage.getGateway());
        assertNull(m_editGatewayPage.getGatewayId());
    }

    public void testPageBeginRenderEdit() {

        Integer id = new Integer(5);
        Gateway gateway = new Gateway(id);
        gateway.setName("kuku");

        MockControl contextControl = MockControl.createStrictControl(DialPlanContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        DialPlanContext context = (DialPlanContext) contextControl.getMock();
        contextControl.expectAndReturn(context.getGateway(id), gateway);
        contextControl.replay();

        m_editGatewayPage.setDialPlanManager(context);
        m_editGatewayPage.setGatewayId(id);
        m_editGatewayPage.pageBeginRender(null);

        contextControl.verify();

        assertEquals(id, m_editGatewayPage.getGatewayId());
        assertEquals("kuku", m_editGatewayPage.getGateway().getName());
        assertNotSame(gateway, m_editGatewayPage.getGateway());
    }
}
