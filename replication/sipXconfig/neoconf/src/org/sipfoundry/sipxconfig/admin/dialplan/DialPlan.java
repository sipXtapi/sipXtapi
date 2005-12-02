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
        DialingRule[] rules = getDialingRuleByType(m_rules, InternalRule.class);
        for (int i = 0; i < rules.length; i++) {
            ((InternalRule) rules[i]).setAutoAttendant(operator);
        }
    }
    
    /**
     * There can be multiple internal dialing rules and therefore multiple voicemail
     * extensions, but pick the most likely one.
     */
    public String getLikelyVoiceMailValue() {
        DialingRule[] rules = getDialingRuleByType(m_rules, InternalRule.class);
        if (rules.length == 0) {
            return InternalRule.DEFAULT_VOICEMAIL;
        }

        // return first, it's the most likely
        String voicemail = ((InternalRule) rules[0]).getVoiceMail();
        return voicemail;
    }
    
    static DialingRule[] getDialingRuleByType(List rulesCandidates, Class c) {
        List rules = new ArrayList();
        for (Iterator i = rulesCandidates.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            if (rule.getClass().isAssignableFrom(c)) {
                rules.add(rule);
            }
        }                       

        return (DialingRule[]) rules.toArray(new DialingRule[rules.size()]);
    }
}
