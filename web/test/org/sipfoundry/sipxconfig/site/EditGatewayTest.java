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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanManager;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;

import junit.framework.TestCase;

/**
 * EditGatewayTest
 */
public class EditGatewayTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();

    public void testAddNewGateway() {
        // TODO: these should be mocked
        DialPlanManager manager = new DialPlanManager();
        Gateway gateway = new Gateway();
        gateway.setName("testName");

        EditGateway editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
        editGatewayPage.setDialPlanManager(manager);
        editGatewayPage.setGateway(gateway);
        editGatewayPage.setAddMode(true);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();

        // return to list gateways page in normal case
        cycle.activate(ListGateways.PAGE);
        cycleControl.replay();
        editGatewayPage.saveValid(cycle);
        cycleControl.verify();

        assertEquals(1, manager.getGateways().size());
        Object addedGateway = manager.getGateways().get(0);
        assertEquals(gateway, addedGateway);
    }

    public void testSaveAndAssign() {
        // TODO: these should be mocked
        DialPlan plan = new DialPlan();
        plan.setName("dial plan name");
        DialPlanManager manager = new DialPlanManager();
        manager.addDialPlan(plan);
        Gateway gateway = new Gateway();
        gateway.setName("testName");

        EditGateway editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
        editGatewayPage.setDialPlanManager(manager);
        editGatewayPage.setGateway(gateway);
        editGatewayPage.setCurrentDialPlanId(plan.getId());
        editGatewayPage.setAddMode(true);

        EditDialPlan editDialPlanPage = (EditDialPlan) m_pageMaker
                .getInstance(EditDialPlan.class);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(EditDialPlan.PAGE), editDialPlanPage);
        cycle.activate(editDialPlanPage);
        cycleControl.replay();
        editGatewayPage.saveValid(cycle);
        cycleControl.verify();

        DialPlan editedPlan = editDialPlanPage.getDialPlan();
        assertEquals(plan, editedPlan);

        assertEquals(1, manager.getGateways().size());
        Object addedGateway = manager.getGateways().get(0);
        assertEquals(gateway, addedGateway);

        assertEquals(0, plan.getEmergencyGateways().size());
        assertEquals(1, plan.getGateways().size());
        assertTrue(plan.getGateways().contains(gateway));
    }

    public void testSaveAndAssignEmergencyGateway() {
        // TODO: these should be mocked
        DialPlan plan = new DialPlan();
        plan.setName("dial plan name");
        DialPlanManager manager = new DialPlanManager();
        manager.addDialPlan(plan);
        Gateway gateway = new Gateway();
        gateway.setName("testName");

        EditGateway editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
        editGatewayPage.setDialPlanManager(manager);
        editGatewayPage.setGateway(gateway);
        editGatewayPage.setCurrentDialPlanId(plan.getId());
        editGatewayPage.setAddMode(true);
        editGatewayPage.setEmergencyGateway(true);

        EditDialPlan editDialPlanPage = (EditDialPlan) m_pageMaker
                .getInstance(EditDialPlan.class);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(EditDialPlan.PAGE), editDialPlanPage);
        cycle.activate(editDialPlanPage);
        cycleControl.replay();
        editGatewayPage.saveValid(cycle);
        cycleControl.verify();

        assertEquals(1, manager.getGateways().size());
        Object addedGateway = manager.getGateways().get(0);
        assertEquals(gateway, addedGateway);

        assertEquals(0, plan.getGateways().size());
        assertEquals(1, plan.getEmergencyGateways().size());
        assertTrue(plan.getEmergencyGateways().contains(gateway));
    }

    public void testCancel() {
        EditGateway editGatewayPage = (EditGateway) m_pageMaker.getInstance(EditGateway.class);
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();

        // return to list gateways page in normal case
        cycle.activate(ListGateways.PAGE);
        cycleControl.replay();
        editGatewayPage.cancel(cycle);
        cycleControl.verify();

        // return to dial plan page when dial plan is set
        DialPlan plan = new DialPlan();
        editGatewayPage.setCurrentDialPlanId(plan.getId());
        cycleControl.reset();
        cycle.activate(EditDialPlan.PAGE);
        cycleControl.replay();
        editGatewayPage.cancel(cycle);
        cycleControl.verify();
    }

}
