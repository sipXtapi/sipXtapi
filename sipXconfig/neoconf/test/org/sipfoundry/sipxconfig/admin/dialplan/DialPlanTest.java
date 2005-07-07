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
}
