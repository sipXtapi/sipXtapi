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
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

/**
 * FlexibleDialPlan - dial plan consisting of list of dialing rules
 */
public class FlexibleDialPlan {
    private List m_rules = new ArrayList();

    public boolean addRule(IDialingRule rule) {
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

    private DialingRule getOrgRule(Integer id) {
        int i = m_rules.indexOf(new DialingRule(id));
        if (i < 0) {
            return null;
        }
        return (DialingRule) m_rules.get(i);
    }

    public DialingRule getRule(Integer id) {
        final DialingRule rule = getOrgRule(id);
        return null != rule ? (DialingRule) rule.detach() : null;
    }

    public void deleteRules(Collection selectedRows) {
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            m_rules.remove(new DialingRule(id));
        }
    }

    public boolean updateRule(Integer id, DialingRule rule) {
        // TODO: this is naive implementation - review after adding hibernate
        final DialingRule orgRule = getOrgRule(id);
        if (null == orgRule) {
            return false;
        }
        orgRule.update(rule);
        return true;
    }
}
