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
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;

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

    public void testGetAvailableGateways() {
        Collection availableGateways = m_manager.getAvailableGateways(new Integer(-1));
        assertEquals(0, availableGateways.size());

        DialingRule rule1 = new CustomDialingRule();
        DialingRule rule2 = new CustomDialingRule();
        m_manager.getFlexDialPlan().addRule(rule1);
        m_manager.getFlexDialPlan().addRule(rule2);
        availableGateways = m_manager.getAvailableGateways(rule1.getId());
        assertEquals(0, availableGateways.size());
        availableGateways = m_manager.getAvailableGateways(rule1.getId());
        assertEquals(0, availableGateways.size());

        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();
        Gateway g3 = new Gateway();
        m_manager.addGateway(g1);
        m_manager.addGateway(g2);
        m_manager.addGateway(g3);

        rule1.addGateway(g2);
        rule2.addGateway(g1);
        rule2.addGateway(g3);

        availableGateways = m_manager.getAvailableGateways(rule1.getId());
        assertEquals(2, availableGateways.size());
        assertTrue(availableGateways.contains(g1));
        assertFalse(availableGateways.contains(g2));
        assertTrue(availableGateways.contains(g3));

        availableGateways = m_manager.getAvailableGateways(rule2.getId());
        assertEquals(1, availableGateways.size());
        assertFalse(availableGateways.contains(g1));
        assertTrue(availableGateways.contains(g2));
        assertFalse(availableGateways.contains(g3));
    }

    public void testActivateDialPlan() throws Exception {
        DialPlanManager manager = new DialPlanManager();
        final ConfigGenerator g1 = manager.getGenerator();
        final ConfigGenerator g2 = manager.generateDialPlan();
        final ConfigGenerator g3 = manager.getGenerator();
        assertNotNull(g1);
        assertNotNull(g2);
        assertNotSame(g1,g2);
        assertSame(g2,g3);
    }
}
