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

import java.util.Collection;

import junit.framework.TestCase;

/**
 * FlexibleDialPlanTest
 */
public class FlexibleDialPlanTest extends TestCase {

    public void testAddRule() {
        DialingRule r1 = new DialingRule();
        IDialingRule r2 = new DialingRule();
        Integer id1 = r1.getId();
        
        FlexibleDialPlan plan = new FlexibleDialPlan();
        assertTrue(plan.addRule(r1));
        assertFalse(plan.addRule(r1));
        assertEquals(1,plan.getRules().size());        
        assertTrue(plan.addRule(r2));
        assertEquals(2,plan.getRules().size());
        
        assertTrue(plan.removeRule(id1));
        Collection rules = plan.getRules();
        assertTrue(rules.contains(r2));
        assertFalse(rules.contains(r1));
        assertEquals(1,rules.size());        
    }

}
