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

import java.util.ArrayList;
import java.util.List;

/**
 * FlexibleDialPlan - dial plan consisting of list of dialing rules
 */
public class FlexibleDialPlan {
    private List m_rules = new ArrayList();

    boolean addRule(IDialingRule rule) {
        if (m_rules.contains(rule)) {
            return false;
        }
        m_rules.add(rule);
        return true;
    }

    boolean removeRule(Integer id) {
        return m_rules.remove(new DialingRule(id));
    }

    public List getRules() {
        return m_rules;
    }
}
