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
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlan;

/**
 * ListDialPlansTest
 */
public class ListDialPlansTest extends TestCase {
    private AbstractInstantiator m_pageMaker = new AbstractInstantiator();

    public void testAdd() {
        EditDialPlan editPage = (EditDialPlan) m_pageMaker.getInstance(EditDialPlan.class);
        ListDialPlans listPage = (ListDialPlans) m_pageMaker.getInstance(ListDialPlans.class);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(EditDialPlan.PAGE), editPage);
        cycle.activate(editPage);
        cycleControl.replay();
        listPage.add(cycle);
        cycleControl.verify();

        assertNull(editPage.getDialPlanId());
    }

    public void testEdit() {
        EditDialPlan editPage = (EditDialPlan) m_pageMaker.getInstance(EditDialPlan.class);
        ListDialPlans listPage = (ListDialPlans) m_pageMaker.getInstance(ListDialPlans.class);
        DialPlan plan = new DialPlan();
        listPage.setCurrentRow(plan);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(EditDialPlan.PAGE), editPage);
        cycle.activate(editPage);
        cycleControl.replay();
        listPage.edit(cycle);
        cycleControl.verify();

        assertNotNull(editPage.getDialPlanId());
        assertEquals(plan.getId(), editPage.getDialPlanId());
    }
}
