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
import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * DialPlanContextImpl is an implementation of DialPlanContext with hibernate support.
 */
public class DialPlanContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware,
        DialPlanContext, ApplicationListener {

    private static final String NAME_PROP_NAME = "name";
    private static final String EXTENSION_PROP_NAME = "extension";
    private static final String OPERATOR_CONSTANT = "operator";
    private static final String QUERY_DIALING_RULE_IDS_WITH_NAME = "dialingRuleIdsWithName";

    private class NameInUseException extends UserException {
        private static final String ERROR = "The name \"{1}\" is already in use. "
                + "Please choose another name for this {0}.";

        public NameInUseException(String objectType, String name) {
            super(ERROR, objectType, name);
        }
    }

    private String m_configDirectory;

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
        // Check for duplicate names before saving the rule
        String name = rule.getName();
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), rule,
                QUERY_DIALING_RULE_IDS_WITH_NAME, name, new NameInUseException("dialing rule",
                        name));

        // Save the rule. If it's a new rule then attach it to the dial plan first
        // and save it via the dial plan.
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
        List rules = dialPlan.getRules();
        Collection selectedRules = DataCollectionUtil.findByPrimaryKey(rules, selectedRows
                .toArray());
        for (Iterator i = selectedRules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();

            // Create a copy of the rule with a unique name
            DialingRule ruleDup = (DialingRule) duplicateBean(rule,
                    QUERY_DIALING_RULE_IDS_WITH_NAME);

            rules.add(ruleDup);
        }
        DataCollectionUtil.updatePositions(rules);
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
        getHibernateTemplate().delete(getEmergencyRouting());
        
        DialPlan dialPlan = getDialPlan();
        // unload all rules
        getHibernateTemplate().delete(dialPlan);

        // Flush the session to cause the delete to take immediate effect.
        // Otherwise we can get name collisions on dialing rules when we load the
        // default dial plan, causing a DB integrity exception, even though the
        // collisions would go away as soon as the session was flushed.
        getHibernateTemplate().flush();

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

    private class ExtensionInUseException extends UserException {
        private static final String ERROR = "Extension {0} is already in use. "
                + "Please choose another extension for this auto attendant.";

        public ExtensionInUseException(String extension) {
            super(ERROR, extension);
        }
    }

    public void storeAutoAttendant(AutoAttendant aa) {
        // Check for duplicate names or extensions before saving the call group
        String name = aa.getName();
        final String aaTypeName = "auto attendant";
        DaoUtils.checkDuplicates(getHibernateTemplate(), aa, NAME_PROP_NAME,
                new NameInUseException(aaTypeName, name));
        String extension = aa.getExtension();
        DaoUtils.checkDuplicates(getHibernateTemplate(), aa, EXTENSION_PROP_NAME,
                new ExtensionInUseException(extension));

        getHibernateTemplate().saveOrUpdate(aa);
    }

    public AutoAttendant getOperator() {
        String operatorQuery = "from AutoAttendant a where a.systemId = :operator";
        List operatorList = getHibernateTemplate().findByNamedParam(operatorQuery,
                OPERATOR_CONSTANT, AutoAttendant.OPERATOR_ID);

        AutoAttendant operator = (AutoAttendant) DaoUtils.requireOneOrZero(operatorList,
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
        generator.generate(getEmergencyRouting());
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

    public void applyEmergencyRouting(EmergencyRouting er) {
        getHibernateTemplate().saveOrUpdate(er);
        try {
            EmergencyRoutingRules rules = new EmergencyRoutingRules();
            rules.generate(er, m_coreContext.getDomainName());
            rules.writeToFile(m_configDirectory);
        } catch (IOException e) {
            throw new RuntimeException("Application of emergency routing rules failed.", e);
        }
    }

    public EmergencyRouting getEmergencyRouting() {
        List ers = getHibernateTemplate().loadAll(EmergencyRouting.class);
        if (ers.isEmpty()) {
            EmergencyRouting er = new EmergencyRouting();
            getHibernateTemplate().save(er);
            return er;
        }
        return (EmergencyRouting) ers.get(0);
    }

    public void removeRoutingException(Serializable routingExceptionId) {
        RoutingException re = (RoutingException) getHibernateTemplate().load(
                RoutingException.class, routingExceptionId);
        getEmergencyRouting().removeException(re);
        getHibernateTemplate().saveOrUpdate(re);
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

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask dbEvent = (InitializationTask) event;
            if (dbEvent.getTask().equals("dial-plans")) {
                resetToFactoryDefault();
            } else if (dbEvent.getTask().equals(OPERATOR_CONSTANT)) {
                createOperator();
            }
        }
    }

    void createOperator() {
        AutoAttendant operator = getOperator();
        if (operator == null) {
            operator = AutoAttendant.createOperator();
            storeAutoAttendant(operator);
            DialPlan dialPlan = getDialPlan();
            dialPlan.setOperator(operator);
            getHibernateTemplate().saveOrUpdate(dialPlan);
        }
    }

    public String getVoiceMail() {
        return getDialPlan().getLikelyVoiceMailValue();
    }
}
