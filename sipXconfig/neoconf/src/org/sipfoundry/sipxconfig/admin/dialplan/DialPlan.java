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

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;

public class DialPlan extends BeanWithId {
    private List m_rules = new ArrayList();

    public List getRules() {
        return m_rules;
    }

    public void setRules(List rules) {
        m_rules = rules;
    }

    public void removeRule(DialingRule rule) {
        Object[] keys = new Object[] {
            rule.getId()
        };
        DataCollectionUtil.removeByPrimaryKey(m_rules, keys);
    }

    public void removeRules(Collection ids) {
        DataCollectionUtil.removeByPrimaryKey(m_rules, ids.toArray());
    }

    public void addRule(DialingRule rule) {
        m_rules.add(rule);
        DataCollectionUtil.updatePositions(m_rules);
    }

    public void duplicateRules(Collection selectedRows) {
        Collection rules = DataCollectionUtil.findByPrimaryKey(m_rules, selectedRows.toArray());
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            BeanWithId ruleDup = rule.duplicate();
            m_rules.add(ruleDup);
        }
        DataCollectionUtil.updatePositions(m_rules);
    }

    public void removeAllRules() {
        m_rules.clear();
    }

    public void moveRules(Collection selectedRows, int step) {
        DataCollectionUtil.moveByPrimaryKey(m_rules, selectedRows.toArray(), step);
    }

    public List getGenerationRules() {
        List generationRules = new ArrayList();
        for (Iterator i = m_rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.appendToGenerationRules(generationRules);
        }
        return generationRules;
    }
    
   
    /**
     * Run thru dialing rules and set rellevant dial plans that take
     */
    public void setOperator(AutoAttendant operator) {
        for (Iterator i = m_rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            if (rule.getClass().isAssignableFrom(InternalRule.class)) {
                InternalRule internal = (InternalRule) rule;
                internal.setAutoAttendant(operator);
            }
        }        
    }
}
