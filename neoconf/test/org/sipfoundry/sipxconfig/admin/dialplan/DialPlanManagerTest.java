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

import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;

/**
 * DialPlanManagerTest
 */
public class DialPlanManagerTest extends TestCase {
    public void testActivateDialPlan() throws Exception {
        MockControl controlPlan = MockControl.createStrictControl(FlexibleDialPlanContext.class);
        FlexibleDialPlanContext emptyPlan = (FlexibleDialPlanContext) controlPlan.getMock();
        emptyPlan.getGenerationRules();
        controlPlan.setReturnValue(Collections.EMPTY_LIST, 2);
        controlPlan.replay();

        DialPlanManager manager = new DialPlanManager();
        manager.setFlexDialPlan(emptyPlan);

        final ConfigGenerator g1 = manager.getGenerator();
        final ConfigGenerator g2 = manager.generateDialPlan();
        final ConfigGenerator g3 = manager.getGenerator();
        assertNotNull(g1);
        assertNotNull(g2);
        assertNotSame(g1, g2);
        assertSame(g2, g3);

        controlPlan.verify();
    }
}
