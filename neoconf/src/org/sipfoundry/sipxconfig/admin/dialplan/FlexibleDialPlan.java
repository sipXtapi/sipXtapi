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

import java.util.Collection;
import java.util.List;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * FlexibleDialPlan - dial plan consisting of list of dialing rules
 */
class FlexibleDialPlan extends HibernateDaoSupport implements BeanFactoryAware,
        FlexibleDialPlanContext {

    private BeanFactory m_beanFactory;

    /**
     * Loads dial plan, creates a new one if none exist
     * 
     * @return the single instance of dial plan
     */
    private DialPlan getDialPlan() {
        List list = getHibernateTemplate().loadAll(DialPlan.class);
        if (!list.isEmpty()) {
            return (DialPlan) list.get(0);
        }
        DialPlan plan = new DialPlan();
        getHibernateTemplate().save(plan);
        return plan;
    }

    public void storeRule(DialingRule rule) {
        if (rule.isNew()) {
            DialPlan dialPlan = getDialPlan();
            dialPlan.addRule(rule);
            getHibernateTemplate().saveOrUpdate(dialPlan);
        } else {
            getHibernateTemplate().saveOrUpdate(rule);
        }
    }

    public List getRules() {
        return getDialPlan().getRules();
    }

    public DialingRule getRule(Integer id) {
        return (DialingRule) getHibernateTemplate().load(DialingRule.class, id);
    }

    public void deleteRules(Collection selectedRows) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.removeRules(selectedRows);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public void duplicateRules(Collection selectedRows) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.duplicateRules(selectedRows);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public List getGenerationRules() {
        DialPlan dialPlan = getDialPlan();
        return dialPlan.getGenerationRules();
    }

    /**
     * Resets the flexible dial plan to factory defaults.
     * 
     * Loads default rules definition from bean factory file.
     */
    public void resetToFactoryDefault() {
        DialPlan dialPlan = getDialPlan();
        // unload all rules
        getHibernateTemplate().delete(dialPlan);
        dialPlan = (DialPlan) m_beanFactory.getBean("defaultDialPlan");
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void moveRules(Collection selectedRows, int step) {
        DialPlan dialPlan = getDialPlan();
        dialPlan.moveRules(selectedRows, step);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }
}
