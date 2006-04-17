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
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.sipfoundry.sipxconfig.admin.ExtensionInUseException;
import org.sipfoundry.sipxconfig.admin.NameInUseException;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.SpecialAutoAttendantMode;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

/**
 * DialPlanContextImpl is an implementation of DialPlanContext with hibernate support.
 */
public class DialPlanContextImpl extends SipxHibernateDaoSupport implements BeanFactoryAware,
        DialPlanContext, ApplicationListener {
    private static final String DIALING_RULE_IDS_WITH_NAME_QUERY = "dialingRuleIdsWithName";

    private static final String OPERATOR_CONSTANT = "operator";

    private static final String VALUE = "value";

    private static final String AUTO_ATTENDANT = "auto attendant";

    private static final String DIALING_RULE = "dialing rule";

    private transient ConfigGenerator m_generator;

    private DialingRuleFactory m_ruleFactory;

    private CoreContext m_coreContext;

    private AliasManager m_aliasManager;

    private BeanFactory m_beanFactory;

    private SipxReplicationContext m_sipxReplicationContext;

    private SettingDao m_settingDao;

    private String m_scriptsDirectory;
    
    private Setting m_attendantSettingModel;

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

    /**
     * @param rule new DialingRule to be added to the plan
     * @param position index of a new rule to be added, -1 means append the rule
     */
    public void addRule(int position, DialingRule rule) {
        if (!rule.isNew()) {
            throw new IllegalArgumentException("addRule method can be only called for new rules");
        }
        validateRule(rule);
        DialPlan dialPlan = getDialPlan();
        dialPlan.addRule(position, rule);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public void storeRule(DialingRule rule) {
        validateRule(rule);

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

    /**
     * Checks for duplicate names. Should be called before saving the rule.
     * 
     * @param rule to be verified
     */
    private void validateRule(DialingRule rule) {
        String name = rule.getName();
        DaoUtils.checkDuplicatesByNamedQuery(getHibernateTemplate(), rule,
                DIALING_RULE_IDS_WITH_NAME_QUERY, name,
                new NameInUseException(DIALING_RULE, name));

        // For internal rules, check for alias collisions. Note: this method throws
        // an exception if it finds a duplicate.
        if (rule instanceof InternalRule) {
            checkAliasCollisionsForInternalRule((InternalRule) rule);
        }
        if (rule instanceof AttendantRule) {
            checkAliasCollisionsForAttendantRule((AttendantRule) rule);
        }
    }

    private void checkAliasCollisionsForInternalRule(InternalRule rule) {
        String voiceMailExtension = rule.getVoiceMail();
        if (!m_aliasManager.canObjectUseAlias(rule, voiceMailExtension)) {
            throw new ExtensionInUseException("voicemail", voiceMailExtension);
        }
    }

    private void checkAliasCollisionsForAttendantRule(AttendantRule ar) {
        String attendantExtension = ar.getExtension();
        if (!m_aliasManager.canObjectUseAlias(ar, attendantExtension)) {
            throw new ExtensionInUseException(DIALING_RULE, attendantExtension);
        }

        String aa = ar.getAttendantAliases();
        String[] aliases = AttendantRule.getAttendantAliasesAsArray(aa);

        for (int i = 0; i < aliases.length; i++) {
            String ruleAlias = aliases[i];
            if (!m_aliasManager.canObjectUseAlias(ar, ruleAlias)) {
                final String message = "Alias \"{0}\" is already in use.  "
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
                    DIALING_RULE_IDS_WITH_NAME_QUERY);

            rules.add(ruleDup);
        }
        DataCollectionUtil.updatePositions(rules);
        getHibernateTemplate().saveOrUpdate(dialPlan);
    }

    public List getGenerationRules() {
        DialPlan dialPlan = getDialPlan();
        return dialPlan.getGenerationRules();
    }

    public List getAttendantRules() {
        DialPlan dialPlan = getDialPlan();
        return dialPlan.getAttendantRules();
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
        if (!m_aliasManager.canObjectUseAlias(aa, name)) {
            throw new NameInUseException(AUTO_ATTENDANT, name);
        }
        clearUnsavedValueStorage(aa.getValueStorage());
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

        attendant.setValueStorage(clearUnsavedValueStorage(attendant.getValueStorage()));
        getHibernateTemplate().refresh(attendant);

        Collection attendantRules = getHibernateTemplate().loadAll(AttendantRule.class);
        Collection affectedRules = new ArrayList();
        for (Iterator i = attendantRules.iterator(); i.hasNext();) {
            AttendantRule rule = (AttendantRule) i.next();
            if (rule.checkAttendant(attendant)) {
                affectedRules.add(rule);
            }
        }
        if (!affectedRules.isEmpty()) {
            throw new AttendantInUseException(affectedRules);
        }

        getHibernateTemplate().delete(attendant);
        File script = new File(scriptsDir + '/' + attendant.getScriptFileName());
        if (script.exists()) {
            script.delete();
        }
    }

    public void specialAutoAttendantMode(boolean enabled, AutoAttendant attendant) {
        if (enabled && attendant == null) {
            throw new UserException("Select special auto attendant to be used.");
        }
        m_sipxReplicationContext.replicate(new SpecialAutoAttendantMode(enabled, attendant));
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
        ConfigGenerator generator = (ConfigGenerator) m_beanFactory.getBean(
                ConfigGenerator.BEAN_NAME, ConfigGenerator.class);
        generator.generate(this, getEmergencyRouting());
        m_generator = generator;
        return m_generator;
    }

    public void activateDialPlan() {
        ConfigGenerator generator = getGenerator();
        generator.activate(m_sipxReplicationContext, m_scriptsDirectory);
        // notify the world we are done with activating dial plan
        m_sipxReplicationContext.publishEvent(new DialPlanActivatedEvent(this));
    }

    public void applyEmergencyRouting() {
        EmergencyRoutingRules rules = new EmergencyRoutingRules();
        rules.generate(getEmergencyRouting(), m_coreContext.getDomainName());
        m_sipxReplicationContext.replicate(rules);
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

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setScriptsDirectory(String scriptsDirectory) {
        m_scriptsDirectory = scriptsDirectory;
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
            operator.addGroup(getDefaultAutoAttendantGroup());
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
     * Implement AliasOwner.isAliasInUse. DialPlanContextImpl owns aliases for auto attendants and
     * voicemail.
     */
    public boolean isAliasInUse(String alias) {
        if (getAutoAttendantsWithName(alias).size() > 0) {
            // Look for the ID of an auto attendant with the specified alias/extension.
            // If there is one, then the alias is in use.
            return true;
        }
        if (getInternalRulesWithVoiceMailExtension(alias).size() > 0) {
            return true;
        }
        if (getAttendantRulesWithExtension(alias).size() > 0) {
            return true;
        }
        return isAutoAttendantAliasInUse(alias);
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection bids = new ArrayList();

        Collection autoAttendants = getAutoAttendantsWithName(alias);
        bids.addAll(BeanId.createBeanIdCollection(autoAttendants, AutoAttendant.class));

        Collection internalRules = getInternalRulesWithVoiceMailExtension(alias);
        bids.addAll(BeanId.createBeanIdCollection(internalRules, InternalRule.class));

        Collection attendantRules = getAttendantRulesWithExtension(alias);
        bids.addAll(BeanId.createBeanIdCollection(attendantRules, AttendantRule.class));

        bids.addAll(getBeanIdsOfRulesWithAutoAttendantAlias(alias));

        return bids;
    }

    private Collection getAutoAttendantsWithName(String alias) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam("autoAttendantIdsWithName",
                VALUE, alias);
    }

    private Collection getBeanIdsOfRulesWithAutoAttendantAlias(String alias) {
        Collection objs = getHibernateTemplate().findByNamedQuery(
                "attendantRuleIdsAndAttendantAliases");
        Collection bids = new ArrayList();
        for (Iterator iter = objs.iterator(); iter.hasNext();) {
            Object[] idAndAliases = (Object[]) iter.next();
            Integer id = (Integer) idAndAliases[0];
            String aa = (String) idAndAliases[1];
            String[] aliases = AttendantRule.getAttendantAliasesAsArray(aa);
            if (ArrayUtils.contains(aliases, alias)) {
                bids.add(new BeanId(id, AttendantRule.class));
            }
        }
        return bids;
    }

    private boolean isAutoAttendantAliasInUse(String alias) {
        // Because auto attendant aliases are stored together in a space-delimited string,
        // we can't query the DB for individual aliases. However, there will be so few
        // of these aliases (one string per internal dialing rule) that we can simply load
        // all such alias strings and check them in Java.
        List aliasStrings = getHibernateTemplate().findByNamedQuery("aaAliases");
        for (Iterator iter = SipxCollectionUtils.safeIterator(aliasStrings); iter.hasNext();) {
            String[] aliases = AttendantRule.getAttendantAliasesAsArray((String) iter.next());
            if (ArrayUtils.contains(aliases, alias)) {
                return true;
            }
        }
        return false;
    }

    private Collection getInternalRulesWithVoiceMailExtension(String extension) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam(
                "internalRuleIdsWithVoiceMailExtension", VALUE, extension);
    }

    private Collection getAttendantRulesWithExtension(String extension) {
        return getHibernateTemplate().findByNamedQueryAndNamedParam(
                "attendantRuleIdsWithExtension", VALUE, extension);
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public Group getDefaultAutoAttendantGroup() {
        return m_settingDao.getGroupCreateIfNotFound("auto_attendant", "default");
    }

    public AutoAttendant newAutoAttendantWithDefaultGroup() {
        AutoAttendant aa = (AutoAttendant) m_beanFactory.getBean(AutoAttendant.BEAN_NAME,
                AutoAttendant.class);

        // All auto attendants share same group: default
        Set groups = aa.getGroups();
        if (groups == null || groups.size() == 0) {
            aa.addGroup(getDefaultAutoAttendantGroup());
        }

        return aa;
    }

    public void setAttendantSettingModel(Setting attendantSettingModel) {
        m_attendantSettingModel = attendantSettingModel;
    }
    
    public Setting getAttendantSettingModel() {
        // return copy so original model stays intact
        return m_attendantSettingModel.copy();
    }
}
