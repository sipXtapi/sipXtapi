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

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import junit.framework.TestCase;

/**
 * DialingRuleTest
 */
public class DialingRuleTest extends TestCase {
    public void testDetach() {
        CustomDialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");
        DialingRule detachedRule = (DialingRule) orgRule.detach();
        assertFalse(orgRule.equals(detachedRule));
        assertEquals(orgRule.getDescription(), detachedRule.getDescription());
        assertEquals(orgRule.getName(), detachedRule.getName());
    }

    public void testUpdate() {
        DialingRule orgRule = new CustomDialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        DialingRule newRule = new CustomDialingRule();
        Integer id = newRule.getId();
        assertFalse(id.equals(orgRule.getId()));
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
        rules.add(orgRule);
        rules.add(orgRule.duplicate());
        rules.add(orgRule.duplicate());
        assertEquals(3, rules.size());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            assertEquals("name", rule.getName());
            assertEquals("description", rule.getDescription());
        }
    }

}
