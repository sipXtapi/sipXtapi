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

import junit.framework.TestCase;

public class DialPlanTest extends TestCase {

    public void testSetOperator() {
        DialPlan plan = new DialPlan();
        InternalRule irule = new InternalRule();
        plan.addRule(irule);
        assertNull(irule.getAutoAttendant());
        AutoAttendant operator = new AutoAttendant();
        plan.setOperator(operator);
        assertSame(operator, irule.getAutoAttendant());
    }

    public void testGetDialingRuleByType() {
        DialingRule[] candidates = new DialingRule[] {
            new InternalRule(), new CustomDialingRule(), new InternationalRule(),
            new CustomDialingRule()
        };

        DialingRule[] actual = DialPlan.getDialingRuleByType(Arrays.asList(candidates),
                CustomDialingRule.class);
        assertEquals(2, actual.length);
        assertSame(candidates[1], actual[0]);
        assertSame(candidates[3], actual[1]);
    }

    public void testLikelyVoiceMail() {
        DialPlan plan = new DialPlan();
        assertEquals("101", plan.getLikelyVoiceMailValue());

        DialingRule[] rules = new DialingRule[] {
            new CustomDialingRule()
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("101", plan.getLikelyVoiceMailValue());

        InternalRule irule = new InternalRule();
        irule.setVoiceMail("2000");
        rules = new DialingRule[] {
            new CustomDialingRule(), irule
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals("2000", plan.getLikelyVoiceMailValue());
    }

    public void testGetAttendantRules() {
        DialPlan plan = new DialPlan();
        assertTrue(plan.getAttendantRules().isEmpty());
        DialingRule[] rules = new DialingRule[] {
            new AttendantRule(), new CustomDialingRule(), new AttendantRule()
        };
        plan.setRules(Arrays.asList(rules));
        assertEquals(2, plan.getAttendantRules().size());
    }
}
