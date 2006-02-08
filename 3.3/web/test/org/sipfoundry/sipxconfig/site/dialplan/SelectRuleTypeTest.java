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
package org.sipfoundry.sipxconfig.site.dialplan;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.Creator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleType;

/**
 * SelectRuleTypeTest
 */
public class SelectRuleTypeTest extends TestCase {
    private Creator m_pageMaker = new Creator();
    private SelectRuleType m_selectRuleType;

    protected void setUp() throws Exception {
        m_selectRuleType = (SelectRuleType) m_pageMaker.newInstance(SelectRuleType.class);
    }

    public void testNext() {
        verifyPage(DialingRuleType.CUSTOM, "EditCustomDialRule");
        verifyPage(DialingRuleType.INTERNAL, "EditInternalDialRule");
        verifyPage(DialingRuleType.EMERGENCY, "EditEmergencyDialRule");
        //verifyPage(DialingRuleType.RESTRICTED, "EditRestrictedDialRule");
        verifyPage(DialingRuleType.LONG_DISTANCE, "EditLongDistanceDialRule");
        verifyPage(DialingRuleType.LOCAL, "EditLocalDialRule");
    }

    private void verifyPage(DialingRuleType type, String pageName) {
        EditDialRule page = (EditDialRule) m_pageMaker.newInstance(EditDialRule.class);
        page.setRuleId(new Integer(0));

        m_selectRuleType.setRuleType(type);
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getPage(pageName), page);
        cycleControl.setMatcher(MockControl.EQUALS_MATCHER);
        cycle.activate(page);
        cycleControl.replay();
        m_selectRuleType.next(cycle);
        cycleControl.verify();

        assertNull(page.getRuleId());
    }
}
