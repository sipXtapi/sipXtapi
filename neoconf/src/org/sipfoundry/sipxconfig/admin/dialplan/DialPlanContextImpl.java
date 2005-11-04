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
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CollectionUtils;
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
    private static final String OPERATOR_CONSTANT = "operator";
    private static final String VALUE = "value";
    private static final String AUTO_ATTENDANT = "auto attendant";
    private static final String DIALING_RULE = "dialing rule";
    
    private static final String QUERY_DIALING_RULE_IDS_WITH_NAME = "dialingRuleIdsWithName";
    private static final String QUERY_AUTO_ATTENDANT_IDS_WITH_NAME_OR_EXTENSION = "autoAttendantIdsWithNameOrExtension";
    private static final String QUERY_AUTO_ATTENDANT_ALIASES = "aaAliases";
    private static final String QUERY_INTERNAL_RULE_IDS_WITH_VOICE_MAIL_EXTENSION =
        "internalRuleIdsWithVoiceMailExtension";
    private static final String QUERY_INTERNAL_RULE_IDS_AND_AUTO_ATTENDANT_ALIASES =
        "internalRuleIdsAndAaAliases";
    
    private String m_configDirectory;
    private transient ConfigGenerator m_generator;
    private DialingRuleFactory m_ruleFactory;
    private CoreContext m_coreContext;
    private AliasManager m_aliasManager;
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
        if (!m_aliasManager.canObjectUseAlias(rule, name)) {
            throw new NameInUseException(DIALING_RULE, name);
        }

        // For internal rules, check for alias collisions.  Note: this method throws
        // an exception if it finds a duplicate.
        if (rule instanceof InternalRule) {
            checkAliasCollisionsForInternalRule((InternalRule) rule);
        }
        
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

    // Check voicemail extensions and auto attendant aliases
    // for collisions with other aliases in the system.
    // Throw a UserException if there is a collision.
    private void checkAliasCollisionsForInternalRule(InternalRule rule) {
        // Check the voicemail extension
        String voiceMailExtension = rule.getVoiceMail();
        if (!m_aliasManager.canObjectUseAlias(rule, voiceMailExtension)) {
            final String message = "Extension {0} is already in use.  "
                + "Please choose another voicemail extension.";
            throw new UserException(message, voiceMailExtension);
        }
        
        // Check the auto attendant aliases
        String[] aliases = rule.getAttendantAliasesAsArray();
        for (int i = 0; i < aliases.length; i++) {
            String ruleAlias = aliases[i];
            if (!m_aliasManager.canObjectUseAlias(rule, ruleAlias)) {
                final String message = "Alias {0} is already in use.  "
                    + "Please choose another alias for this auto attendant.";
                throw new UserException(message, ruleAlias);
            }            
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

    public void storeAutoAttendant(AutoAttendant aa) {
        // Check for duplicate names or extensions before saving the call group
        String name = aa.getName();
        String extension = aa.getExtension();
        if (!m_aliasManager.canObjectUseAlias(aa, name)) {
            throw new NameInUseException(AUTO_ATTENDANT, name);
        }
        if (!m_aliasManager.canObjectUseAlias(aa, extension)) {
            throw new ExtensionInUseException(AUTO_ATTENDANT, extension);
        }
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

    public void applyEmergencyRouting() {
        try {
            EmergencyRoutingRules rules = new EmergencyRoutingRules();
            rules.generate(getEmergencyRouting(), m_coreContext.getDomainName());
            rules.writeToFile(m_configDirectory);
        } catch (IOException e) {
            throw new RuntimeException("Application of emergency routing rules failed.", e);
        }
    }

    public void storeEmergencyRouting(EmergencyRouting emergencyRouting) {
        getHibernateTemplate().saveOrUpdate(emergencyRouting);
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

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
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

    public void removeGateways(Collection gatewayIds) {
        List rules = getRules();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.removeGateways(gatewayIds);
            storeRule(rule);
        }
        EmergencyRouting emergencyRouting = getEmergencyRouting();
        emergencyRouting.removeGateways(gatewayIds);
        storeEmergencyRouting(emergencyRouting);
        applyEmergencyRouting();
    }

    /**
     * Implement AliasOwner.isAliasInUse.  DialPlanContextImpl owns aliases for
     * auto attendants and voicemail.
     */
    public boolean isAliasInUse(String alias) {
        return isAutoAttendantNameOrExtensionInUse(alias)
                || isAutoAttendantAliasInUse(alias)
                || isVoiceMailExtensionInUse(alias);
    }

    private boolean isAutoAttendantNameOrExtensionInUse(String alias) {
        // Look for the ID of an auto attendant with the specified alias/extension.
        // If there is one, then the alias is in use.
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_AUTO_ATTENDANT_IDS_WITH_NAME_OR_EXTENSION, VALUE, alias);
        return CollectionUtils.safeSize(objs) > 0;        
    }
    
    private boolean isAutoAttendantAliasInUse(String alias) {
        // Because auto attendant aliases are stored together in a comma-delimited string,
        // we can't query the DB for individual aliases.  However, there will be so few
        // of these aliases (one string per internal dialing rule) that we can simply load
        // all such alias strings and check them in Java.
        boolean isAliasInUse = false;
        List aliasStrings = getHibernateTemplate().findByNamedQuery(QUERY_AUTO_ATTENDANT_ALIASES);
        for (Iterator iter = CollectionUtils.safeIterator(aliasStrings); iter.hasNext();) {
            String aliasesString = (String) iter.next();
            isAliasInUse = doesAliasesStringContainAlias(aliasesString, alias);
            if (isAliasInUse) {
                break;
            }
        }
        return isAliasInUse;
    }
    
    private boolean doesAliasesStringContainAlias(String aliasesString, String alias) {
        boolean containsAlias = false;
        String[] aliases = InternalRule.getAttendantAliasesAsArray(aliasesString);
        for (int i = 0; i < aliases.length; i++) {
            String ruleAlias = aliases[i];
            if (ruleAlias.equals(alias)) {
                containsAlias = true;
                break;
            }
        }
        return containsAlias;
    }
    
    private boolean isVoiceMailExtensionInUse(String alias) {
        List objs = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_INTERNAL_RULE_IDS_WITH_VOICE_MAIL_EXTENSION, VALUE, alias);
        return CollectionUtils.safeSize(objs) > 0;        
    }
    
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection bids = new ArrayList();
        bids.addAll(getBeanIdsOfAutoAttendantsWithNameOrExtension(alias));
        bids.addAll(getBeanIdsOfInternalRulesWithAutoAttendantAlias(alias));
        bids.addAll(getBeanIdsOfInternalRulesWithVoiceMailExtension(alias));
        return bids;
    }
    
    private Collection getBeanIdsOfAutoAttendantsWithNameOrExtension(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_AUTO_ATTENDANT_IDS_WITH_NAME_OR_EXTENSION, VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, AutoAttendant.class);
        return bids;
    }
    
    private Collection getBeanIdsOfInternalRulesWithAutoAttendantAlias(String alias) {
        Collection objs = getHibernateTemplate().findByNamedQuery(
                QUERY_INTERNAL_RULE_IDS_AND_AUTO_ATTENDANT_ALIASES);
        Collection bids = new ArrayList();
        for (Iterator iter = objs.iterator(); iter.hasNext();) {
            Object[] idAndAliases = (Object[]) iter.next();
            Integer id = (Integer) idAndAliases[0];
            String aliases = (String) idAndAliases[1];
            if (doesAliasesStringContainAlias(aliases, alias)) {
                bids.add(new BeanId(id, InternalRule.class));
            }
        }
        return bids;
    }

    private Collection getBeanIdsOfInternalRulesWithVoiceMailExtension(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_INTERNAL_RULE_IDS_WITH_VOICE_MAIL_EXTENSION, VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, InternalRule.class);
        return bids;
    }
    
}
