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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;

/**
 * DialingRuleTest
 */
public class DialingRuleTest extends TestCase {
    public void testDetach() throws Exception {
        CustomDialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");
        DialingRule detachedRule = (DialingRule) orgRule.duplicate();
        assertTrue(orgRule.equals(detachedRule));
        assertNotSame(orgRule, detachedRule);
        assertEquals(orgRule.getDescription(), detachedRule.getDescription());
        assertEquals(orgRule.getName(), detachedRule.getName());
    }

    public void testUpdate() {
        DialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        DialingRule newRule = new CustomDialingRule();
        Integer id = newRule.getId();
        // //assertFalse(id.equals(orgRule.getId()));
        newRule.update(orgRule);
        assertEquals(id, newRule.getId());
        assertEquals("name", newRule.getName());
        assertEquals("description", newRule.getDescription());
    }

    public void testDuplicate() {
        DialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        Set rules = new HashSet();
        rules.add(orgRule.setUniqueId());
        rules.add(orgRule.duplicate().setUniqueId());
        rules.add(orgRule.duplicate().setUniqueId());
        assertEquals(3, rules.size());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            assertEquals("name", rule.getName());
            assertEquals("description", rule.getDescription());
        }
    }

    public void testGetAvailableGateways() {
        List allGateways = new ArrayList();
        DialingRule rule1 = new CustomDialingRule();
        DialingRule rule2 = new CustomDialingRule();
        Collection availableGateways = rule1.getAvailableGateways(allGateways);
        assertEquals(0, availableGateways.size());
        availableGateways = rule2.getAvailableGateways(allGateways);
        assertEquals(0, availableGateways.size());

        Gateway g1 = new Gateway();
        g1.setUniqueId();
        Gateway g2 = new Gateway();
        g2.setUniqueId();
        Gateway g3 = new Gateway();
        g3.setUniqueId();
        allGateways.add(g1);
        allGateways.add(g2);
        allGateways.add(g3);

        rule1.addGateway(g2);
        rule2.addGateway(g1);
        rule2.addGateway(g3);

        availableGateways = rule1.getAvailableGateways(allGateways);
        assertEquals(2, availableGateways.size());
        assertTrue(availableGateways.contains(g1));
        assertFalse(availableGateways.contains(g2));
        assertTrue(availableGateways.contains(g3));

        availableGateways = rule2.getAvailableGateways(allGateways);
        assertEquals(1, availableGateways.size());
        assertFalse(availableGateways.contains(g1));
        assertTrue(availableGateways.contains(g2));
        assertFalse(availableGateways.contains(g3));
    }

}
