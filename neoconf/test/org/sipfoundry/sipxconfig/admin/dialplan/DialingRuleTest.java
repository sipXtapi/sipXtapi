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

import junit.framework.TestCase;

/**
 * DialingRuleTest
 */
public class DialingRuleTest extends TestCase {
    public void testDetach() {
        DialingRule orgRule = new DialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");
        DialingRule detachedRule = (DialingRule) orgRule.detach();
        assertFalse(orgRule.equals(detachedRule));
        assertEquals(orgRule.getDescription(), detachedRule.getDescription());
        assertEquals(orgRule.getName(), detachedRule.getName());
    }

    public void testUpdate() {
        DialingRule orgRule = new DialingRule();
        orgRule.setName("name");
        orgRule.setDescription("description");

        DialingRule newRule = new DialingRule();
        Integer id = newRule.getId();
        assertFalse(id.equals(orgRule.getId()));
        newRule.update(orgRule);
        assertEquals(id, newRule.getId());
        assertEquals("name", newRule.getName());
        assertEquals("description", newRule.getDescription());
    }
}
