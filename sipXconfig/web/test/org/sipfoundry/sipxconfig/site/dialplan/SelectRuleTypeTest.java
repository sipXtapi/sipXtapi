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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
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
        verifyPage(DialingRuleType.ATTENDANT, "EditAttendantDialRule");
        verifyPage(DialingRuleType.LONG_DISTANCE, "EditLongDistanceDialRule");
        verifyPage(DialingRuleType.LOCAL, "EditLocalDialRule");
    }

    private void verifyPage(DialingRuleType type, String pageName) {
        EditDialRule page = (EditDialRule) m_pageMaker.newInstance(EditDialRule.class);
        page.setRuleId(new Integer(0));

        m_selectRuleType.setRuleType(type);
        IMocksControl cycleControl = EasyMock.createStrictControl();
        IRequestCycle cycle = cycleControl.createMock(IRequestCycle.class);
        cycle.getPage(pageName);
        cycleControl.andReturn(page);
        cycleControl.replay();

        IPage nextPage = m_selectRuleType.next(cycle);
        assertSame(nextPage, page);

        assertNull(page.getRuleId());

        cycleControl.verify();
    }
}
