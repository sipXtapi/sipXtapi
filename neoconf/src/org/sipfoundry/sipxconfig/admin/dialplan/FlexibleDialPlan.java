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

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.HibernateTemplate;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * FlexibleDialPlan - dial plan consisting of list of dialing rules
 */
class FlexibleDialPlan extends HibernateDaoSupport implements BeanFactoryAware,
        FlexibleDialPlanContext {

    private static final String[] DEFAULT_RULE_NAMES = {
        "defaultEmergencyRule", "defaultInternationalRule", "defaultInternalRule",
        "defaultLocalRule", "defaultLongDistanceRule", "defaultRestrictedRule",
        "defaultTollFreeRule"
    };

    private BeanFactory m_beanFactory;

    public void storeRule(IDialingRule rule) {
        getHibernateTemplate().saveOrUpdate(rule);
    }

    boolean removeRule(Integer id) {
        Object rule = getHibernateTemplate().load(DialingRule.class, id);
        getHibernateTemplate().delete(rule);
        return true;
    }

    public List getRules() {
        return getHibernateTemplate().loadAll(DialingRule.class);
    }

    public DialingRule getRule(Integer id) {
        return (DialingRule) getHibernateTemplate().load(DialingRule.class, id);
    }

    public void deleteRules(Collection selectedRows) {
        Collection rules = new ArrayList();
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            DialingRule rule = getRule(id);
            rules.add(rule);
        }
        getHibernateTemplate().deleteAll(rules);
    }

    public void duplicateRules(Collection selectedRows) {
        HibernateTemplate hibernate = getHibernateTemplate();
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            DialingRule rule = getRule(id);
            BeanWithId ruleDup = rule.duplicate();
            hibernate.save(ruleDup);
        }
    }

    public List getGenerationRules() {
        ArrayList generationRules = new ArrayList();
        List rules = getRules();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.appendToGenerationRules(generationRules);
        }
        return generationRules;
    }

    /**
     * Resets the flexible dial plan to factory defaults.
     * 
     * Loads default rules definition from bean factory file.
     */
    public void resetToFactoryDefault() {
        // unload all rules
        HibernateTemplate hibernate = getHibernateTemplate();
        List allRules = hibernate.loadAll(DialingRule.class);
        hibernate.deleteAll(allRules);

        for (int i = 0; i < DEFAULT_RULE_NAMES.length; i++) {
            String beanName = DEFAULT_RULE_NAMES[i];
            DialingRule rule = (DialingRule) m_beanFactory.getBean(beanName);
            storeRule((IDialingRule) rule.duplicate());
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
