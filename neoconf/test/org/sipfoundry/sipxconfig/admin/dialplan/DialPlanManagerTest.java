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

import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

/**
 * DialPlanManagerTest
 */
public class DialPlanManagerTest extends TestCase {
    private DialPlanManager m_manager;

    protected void setUp() throws Exception {
        m_manager = new DialPlanManager();
    }

    public void testAddGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();

        assertNull(m_manager.getGateway(null));

        // add g1
        assertTrue(m_manager.addGateway(g1));

        assertEquals(1, m_manager.getGateways().size());
        assertTrue(m_manager.getGateways().contains(g1));
        assertFalse(m_manager.getGateways().contains(g2));

        // add g2
        assertTrue(m_manager.addGateway(g2));

        assertEquals(2, m_manager.getGateways().size());
        assertTrue(m_manager.getGateways().contains(g1));
        assertTrue(m_manager.getGateways().contains(g2));

        // add g1 again
        assertFalse(m_manager.addGateway(g1));
    }

    public void testDeleteGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();
        Gateway g3 = new Gateway();

        // add all
        assertTrue(m_manager.addGateway(g1));
        assertTrue(m_manager.addGateway(g2));
        assertTrue(m_manager.addGateway(g3));

        Integer[] toBeRemoved = { g1.getId(), g3.getId() };
        m_manager.deleteGateways(Arrays.asList(toBeRemoved));

        List gateways = m_manager.getGateways();
        
        assertEquals(1, gateways.size());
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));
        assertFalse(gateways.contains(g3));
    }

    public void testUpdateGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();
        m_manager.addGateway(g1);
        m_manager.addGateway(g2);

        Integer id = g1.getId();
        Gateway edited = new Gateway();
        edited.setAddress("a1");
        edited.setName("sd");
        edited.setDescription("ff");

        assertFalse(m_manager.updateGateway(edited.getId(), edited));

        // TODO: use beanutils ?
        // diferent before update
        assertFalse(edited.getName().equals(g1.getName()));
        assertFalse(edited.getAddress().equals(g1.getAddress()));
        assertFalse(edited.getDescription().equals(g1.getDescription()));

        m_manager.updateGateway(id, edited);

        // the same after update
        assertEquals(g1.getAddress(), edited.getAddress());
        assertEquals(g1.getName(), edited.getName());
        assertEquals(g1.getDescription(), edited.getDescription());
    }

    public void testDialPlans() {
        DialPlan p1 = new DialPlan();
        DialPlan p2 = new DialPlan();

        assertNull(m_manager.getDialPlan(null));

        // add p1
        assertTrue(m_manager.addDialPlan(p1));

        assertEquals(1, m_manager.getDialPlans().size());
        assertTrue(m_manager.getDialPlans().contains(p1));
        assertFalse(m_manager.getDialPlans().contains(p2));

        // add p2
        assertTrue(m_manager.addDialPlan(p2));

        assertEquals(2, m_manager.getDialPlans().size());
        assertTrue(m_manager.getDialPlans().contains(p1));
        assertTrue(m_manager.getDialPlans().contains(p2));

        // add p1 again
        assertFalse(m_manager.addDialPlan(p1));
    }

    public void testUpdateDialPlan() {
        DialPlan p1 = new DialPlan();
        DialPlan p2 = new DialPlan();
        m_manager.addDialPlan(p1);
        m_manager.addDialPlan(p2);

        Integer id = p1.getId();
        DialPlan edited = new DialPlan();
        edited.setName("sd");
        edited.setDescription("ff");

        assertFalse(m_manager.updateDialPlan(edited.getId(), edited));

        // TODO: use beanutils ?
        // diferent before update
        assertFalse(edited.getName().equals(p1.getName()));
        assertFalse(edited.getDescription().equals(p1.getDescription()));

        m_manager.updateDialPlan(id, edited);

        // the same after update
        assertEquals(p1.getName(), edited.getName());
        assertEquals(p1.getDescription(), edited.getDescription());
    }

    public void testDeleteDialPlan() {
        DialPlan p1 = new DialPlan();
        DialPlan p2 = new DialPlan();
        DialPlan p3 = new DialPlan();

        // add all
        assertTrue(m_manager.addDialPlan(p1));
        assertTrue(m_manager.addDialPlan(p2));
        assertTrue(m_manager.addDialPlan(p3));

        Integer[] toBeRemoved = { p1.getId(), p3.getId() };
        m_manager.deleteDialPlans(Arrays.asList(toBeRemoved));

        List plans = m_manager.getDialPlans();
        
        assertEquals(1, plans.size());
        assertFalse(plans.contains(p1));
        assertTrue(plans.contains(p2));
        assertFalse(plans.contains(p3));
    }

}
