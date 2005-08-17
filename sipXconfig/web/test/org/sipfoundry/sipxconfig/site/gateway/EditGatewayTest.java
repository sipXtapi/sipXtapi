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
package org.sipfoundry.sipxconfig.site.gateway;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.gateway.ListGateways;

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
        Gateway g = new Gateway();
        g.setUniqueId();

        MockControl contextControl = MockControl.createStrictControl(GatewayContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        GatewayContext context = (GatewayContext) contextControl.getMock();

        context.storeGateway(g);

        contextControl.replay();
        m_editGatewayPage.setGatewayContext(context);
        m_editGatewayPage.setGateway(g);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        m_editGatewayPage.setNextPage(ListGateways.PAGE);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();        
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);

        cycleControl.verify();
        contextControl.verify();
    }

    public void testSaveAndAssign() {
        DialingRule rule = new CustomDialingRule();
        rule.setUniqueId();
        rule.setName("my rule name");

        Gateway g = new Gateway();
        g.setUniqueId();

        MockControl dialPlanContextControl = MockControl
                .createStrictControl(DialPlanContext.class);
        dialPlanContextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        DialPlanContext dialPlanContext = (DialPlanContext) dialPlanContextControl.getMock();

        MockControl contextControl = MockControl.createStrictControl(GatewayContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        GatewayContext context = (GatewayContext) contextControl.getMock();

        context.storeGateway(g);
        dialPlanContext.getRule(rule.getId());
        dialPlanContextControl.setReturnValue(rule);
        dialPlanContext.storeRule(rule);

        dialPlanContextControl.replay();
        contextControl.replay();

        m_editGatewayPage.setDialPlanManager(dialPlanContext);
        m_editGatewayPage.setGatewayContext(context);
        m_editGatewayPage.setGateway(g);
        m_editGatewayPage.setRuleId(rule.getId());
        m_editGatewayPage.setNextPage("EditCustomDialRule");
        m_editGatewayPage.pageBeginRender(null);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.replay();
        m_editGatewayPage.saveValid(cycle);

        cycleControl.verify();
        dialPlanContextControl.verify();
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
        Gateway gateway = new Gateway();
        gateway.setUniqueId();
        gateway.setName("kuku");
        Integer id = gateway.getId();

        MockControl contextControl = MockControl.createStrictControl(GatewayContext.class);
        contextControl.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        GatewayContext context = (GatewayContext) contextControl.getMock();

        contextControl.expectAndReturn(context.getGateway(id), gateway);
        contextControl.replay();

        m_editGatewayPage.setGatewayContext(context);
        m_editGatewayPage.setGatewayId(id);
        m_editGatewayPage.pageBeginRender(null);

        contextControl.verify();

        assertEquals(id, m_editGatewayPage.getGatewayId());
        assertEquals("kuku", m_editGatewayPage.getGateway().getName());
    }
}
