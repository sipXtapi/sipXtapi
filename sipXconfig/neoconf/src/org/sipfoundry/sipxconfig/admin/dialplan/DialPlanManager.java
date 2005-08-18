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

import java.io.File;
import java.io.IOException;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * DialPlanManager is an implementation of DialPlanContext with hibernate support.
 */
public class DialPlanManager extends HibernateDaoSupport 
        implements BeanFactoryAware, DialPlanContext, ApplicationListener {

    private String m_configDirectory;

    private EmergencyRouting m_emergencyRouting = new EmergencyRouting();

    private transient ConfigGenerator m_generator;

    private DialingRuleFactory m_ruleFactory;

    private CoreContext m_coreContext;

    private BeanFactory m_beanFactory;

    /**
     * Loads dial plan, creates a new one if none exist
     * 
     * @return the single instance of dial plan
     */
    DialPlan getDialPlan() {
        List list = getHibernateTemplate().loadAll(DialPlan.class);
        if (!list.isEmpty()) {
            return (DialPlan) list.get(0);
        }
        DialPlan plan = new DialPlan();
        getHibernateTemplate().save(plan);
        return plan;
    }

    public boolean isDialPlanEmpty() {
        boolean empty = getHibernateTemplate().loadAll(DialPlan.class).isEmpty();
        return empty;
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
        AutoAttendant operator = getOperator();
        dialPlan.setOperator(operator);
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

    public void storeAutoAttendant(AutoAttendant aa) {
        getHibernateTemplate().saveOrUpdate(aa);
    }

    public AutoAttendant getOperator() {
        String operatorQuery = "from AutoAttendant a where a.systemId = :operator";
        List operatorList = getHibernateTemplate().findByNamedParam(operatorQuery, "operator",
                AutoAttendant.OPERATOR_ID);

        AutoAttendant operator = (AutoAttendant) CoreContextImpl.requireOneOrZero(operatorList,
                operatorQuery);

        return operator;
    }

    public List getAutoAttendants() {
        List gateways = getHibernateTemplate().loadAll(AutoAttendant.class);
        return gateways;
    }

    public AutoAttendant getAutoAttendant(Integer id) {
        return (AutoAttendant) getHibernateTemplate().load(AutoAttendant.class, id);

    }

    public void deleteAutoAttendantsByIds(Collection attendantIds, String scriptsDir) {
        // TODO: Remove attendants from internal dialing rules
        for (Iterator i = attendantIds.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            AutoAttendant aa = getAutoAttendant(id);
            deleteAutoAttendant(aa, scriptsDir);
        }
    }

    public void deleteAutoAttendant(AutoAttendant attendant, String scriptsDir) {
        if (attendant.isOperator()) {
            throw new AttendantInUseException();
        }
        Collection rules = getRulesUsedByAttendant(attendant);
        if (rules.size() > 0) {
            throw new AttendantInUseException(rules);
        }

        getHibernateTemplate().delete(attendant);
        File script = new File(scriptsDir + '/' + attendant.getScriptFileName());
        if (script.exists()) {
            script.delete();
        }
    }

    public List getRulesUsedByAttendant(AutoAttendant attendant) {
        String query = "from InternalRule r where r.autoAttendant = :attendant";
        List attendants = getHibernateTemplate().findByNamedParam(query, "attendant", attendant);

        return attendants;
    }

    /**
     * This is for testing only. TODO: need to find a better way of cleaning database between
     * tests
     */
    public void clear() {
        resetToFactoryDefault();
        List attendants = getHibernateTemplate().loadAll(AutoAttendant.class);
        getHibernateTemplate().deleteAll(attendants);
    }

    public ConfigGenerator generateDialPlan() {
        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(m_emergencyRouting);
        generator.generate(this);
        m_generator = generator;
        return m_generator;
    }

    public void activateDialPlan() {
        ConfigGenerator generator = getGenerator();
        try {
            generator.activate(m_configDirectory);
        } catch (IOException e) {
            throw new RuntimeException("Activation of Dial Plan incomplete.", e);
        }
    }

    public void applyEmergencyRouting() {
        try {
            EmergencyRoutingRules rules = new EmergencyRoutingRules();
            rules.generate(m_emergencyRouting, m_coreContext.getDomainName());
            rules.writeToFile(m_configDirectory);
        } catch (IOException e) {
            throw new RuntimeException("Application of emergency routing rules failed.", e);
        }
    }

    public ConfigGenerator getGenerator() {
        if (null == m_generator) {
            return generateDialPlan();
        }
        return m_generator;
    }

    public DialingRuleFactory getRuleFactory() {
        return m_ruleFactory;
    }

    public void setRuleFactory(DialingRuleFactory ruleFactory) {
        m_ruleFactory = ruleFactory;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public EmergencyRouting getEmergencyRouting() {
        return m_emergencyRouting;
    }

    public void setEmergencyRouting(EmergencyRouting emergencyRouting) {
        m_emergencyRouting = emergencyRouting;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public Object load(Class c, Integer id) {
        Object o = getHibernateTemplate().load(c, id);
        return o;
    }
    
    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask dbEvent = (InitializationTask) event;            
            if (dbEvent.getTask().equals("dial-plans")) {
                resetToFactoryDefault();
            }
        }
    }
}
