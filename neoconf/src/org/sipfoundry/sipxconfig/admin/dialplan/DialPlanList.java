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

import java.util.ArrayList;
import java.util.List;

/**
 * DialPlanList
 */
public class DialPlanList {
    private List m_dialPlans = new ArrayList();

    public DialPlanList() {
        int plan = 1;
        m_dialPlans.add(new DialPlan("aaa", plan++));
        m_dialPlans.add(new DialPlan("bbb", plan++));
        m_dialPlans.add(new DialPlan("ccc", plan++));
        m_dialPlans.add(new DialPlan("ddd", plan++));
    }

    public List getDialPlans() {
        return m_dialPlans;
    }

    public void setDialPlans(List dialPlans) {
        m_dialPlans = dialPlans;
    }

}
