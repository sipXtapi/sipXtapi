/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Arrays;
import java.util.Collection;
import java.util.Set;

import junit.framework.TestCase;

/**
 * DialPlanTest
 */
public class DialPlanTest extends TestCase {
    public void testAddGateway() {
        Gateway g1 = new Gateway();
        g1.setName("kuku");
        Gateway g2 = new Gateway();
        g2.setName("bongo");

        // adding
        DialPlan plan = new DialPlan();
        final boolean emergency = false;
        assertTrue(plan.addGateway(g1, emergency));
        assertEquals(1, plan.getGateways().size());
        assertFalse(plan.addGateway(g1, emergency));
        assertEquals(1, plan.getGateways().size());
        assertTrue(plan.addGateway(g2, emergency));
        assertEquals(2, plan.getGateways().size());

        Set gateways = plan.getGateways();
        assertTrue(gateways.contains(g1));
        assertTrue(gateways.contains(g2));

        // removing
        assertTrue(plan.removeGateway(g1.getId(), emergency));
        // extremely unlikely that we have -1 gateway
        assertFalse(plan.removeGateway(new Integer(-1), false));
        assertEquals(1, plan.getGateways().size());
        assertFalse(plan.removeGateway(g1.getId(), emergency));
        assertEquals(1, plan.getGateways().size());

        gateways = plan.getGateways();
        assertFalse(gateways.contains(g1));
        assertTrue(gateways.contains(g2));

        // remove the last one
        assertTrue(plan.removeGateway(g2.getId(), emergency));
        assertEquals(0, plan.getGateways().size());
    }

    public void testRemoveGateways() {
        boolean[] types = { false, true };

        for (int i = 0; i < types.length; i++) {
            boolean emergency = types[i];
            Gateway g1 = new Gateway();
            g1.setName("1");
            Gateway g2 = new Gateway();
            g2.setName("2");
            Gateway g3 = new Gateway();
            g3.setName("3");

            Integer[] toBeRemoved = { g1.getId(), g3.getId() };

            DialPlan plan = new DialPlan();

            plan.addGateway(g1, emergency);
            plan.addGateway(g2, emergency);
            plan.addGateway(g3, emergency);

            plan.removeGateways(Arrays.asList(toBeRemoved), emergency);
            Collection gateways;
            if (emergency) {
                gateways = plan.getEmergencyGateways();
            } else {
                gateways = plan.getGateways();
            }

            assertEquals(1, gateways.size());
            assertTrue(gateways.contains(g2));
            assertFalse(gateways.contains(g1));
            assertFalse(gateways.contains(g3));
        }
    }

}
