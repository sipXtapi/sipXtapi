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

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

/**
 * DialPlanManagerTest
 */
public class DialPlanContextTestDb extends TestCase {
    private DialPlanContext m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (DialPlanContext) appContext.getBean(DialPlanContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
    }

    public void testAddGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();

        // add g1
        m_context.addGateway(g1);

        assertEquals(1, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));
        assertFalse(m_context.getGateways().contains(g2));

        // add g2
        m_context.addGateway(g2);

        assertEquals(2, m_context.getGateways().size());
        assertTrue(m_context.getGateways().contains(g1));
        assertTrue(m_context.getGateways().contains(g2));
    }

    public void testDeleteGateway() {
        Gateway g1 = new Gateway();
        Gateway g2 = new Gateway();
        Gateway g3 = new Gateway();

        // add all
        m_context.addGateway(g1);
        m_context.addGateway(g2);
        m_context.addGateway(g3);

        Integer[] toBeRemoved = { g1.getId(), g3.getId() };
        m_context.deleteGateways(Arrays.asList(toBeRemoved));

        List gateways = m_context.getGateways();

        assertEquals(1, gateways.size());
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));
        assertFalse(gateways.contains(g3));
    }

    public void testUpdateGateway() throws Exception {
        TestHelper.setUpHibernateSession();
        Gateway g1 = new Gateway();
        m_context.addGateway(g1);
        Integer id = g1.getId();
        
        m_context.addGateway(new Gateway());

        System.err.println(id);
        Gateway edited = new Gateway();
        edited.setAddress("a1");
        edited.setName("sd");
        edited.setDescription("ff");

        // different before update
        assertFalse(edited.getName().equals(g1.getName()));
        assertFalse(edited.getAddress().equals(g1.getAddress()));
        assertFalse(edited.getDescription().equals(g1.getDescription()));

        m_context.updateGateway(id, edited);

        // the same after update
        assertEquals(g1.getAddress(), edited.getAddress());
        assertEquals(g1.getName(), edited.getName());
        assertEquals(g1.getDescription(), edited.getDescription());
        TestHelper.tearDownHibernateSession();
    }
}
