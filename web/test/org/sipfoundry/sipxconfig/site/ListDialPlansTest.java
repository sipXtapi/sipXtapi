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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;

import junit.framework.TestCase;

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

        assertTrue(editPage.getAddMode());
    }

    public void testEdit() {
        EditDialPlan editPage = (EditDialPlan) m_pageMaker.getInstance(EditDialPlan.class);
        ListDialPlans listPage = (ListDialPlans) m_pageMaker.getInstance(ListDialPlans.class);

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(EditDialPlan.PAGE), editPage);
        cycle.activate(editPage);
        cycleControl.replay();
        listPage.edit(cycle);
        cycleControl.verify();

        assertFalse(editPage.getAddMode());
    }
}
