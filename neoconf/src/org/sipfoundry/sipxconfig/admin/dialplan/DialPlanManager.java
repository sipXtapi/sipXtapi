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

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.Organization;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * DialPlanManager is an implementation of DialPlanContext with hibernate support.
 */
class DialPlanManager extends HibernateDaoSupport implements BeanFactoryAware, 
        DialPlanContext  {
    
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

    public List getGateways() {
        List gateways = getHibernateTemplate().loadAll(Gateway.class);
        return gateways;
    }

    public Gateway getGateway(Integer id) {
        return (Gateway) getHibernateTemplate().load(Gateway.class, id);
        
    }

    public void storeAutoAttendant(AutoAttendant aa) {
        getHibernateTemplate().saveOrUpdate(aa);
    }

    public AutoAttendant getOperator() {
        String operatorQuery = "from AutoAttendant a where a.systemId = :operator";
        List operatorList = getHibernateTemplate().findByNamedParam(operatorQuery, "operator",
                AutoAttendant.OPERATOR_ID);

        AutoAttendant operator = (AutoAttendant) CoreContextImpl.requireOneOrZero(operatorList, operatorQuery);
        
        return operator;
    }

    public List getAutoAttendants() {
        List gateways = getHibernateTemplate().loadAll(AutoAttendant.class);
        return gateways;
    }

    public AutoAttendant getAutoAttendant(Integer id) {
        return (AutoAttendant) getHibernateTemplate().load(AutoAttendant.class, id);
        
    }

    public void deleteAutoAttendantsByIds(Collection attendantIds) {
        // TODO: Remove attendants from internal dialing rules
        for (Iterator i = attendantIds.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            AutoAttendant aa = getAutoAttendant(id);
            deleteAutoAttendant(aa);
        }        
    }
    
    public void deleteAutoAttendant(AutoAttendant attendant) {
        if (attendant.isOperator()) {
            throw new AttendantInUseException("You cannot delete the operator attendant");
        }
        Collection rules = getRulesUsedByAttendant(attendant);
        if (rules.size() > 0) {
            StringBuffer msg = new StringBuffer("The attendant is in use by the following "
                    + "dialing rule(s) and therefore cannot be deleted: ");
            Iterator irules = rules.iterator();
            // FIXME for JDK15
            for (int i = 0; irules.hasNext(); i++) {
                DialingRule rule = (DialingRule) irules.next();
                if (i != 0) {
                    msg.append(", ");
                }
                msg.append(rule.getName());
            }
            
            throw new AttendantInUseException(msg.toString());
        }

        getHibernateTemplate().delete(attendant);
    }
    
    public List getRulesUsedByAttendant(AutoAttendant attendant) {
        String query = "from InternalRule r where r.autoAttendant = :attendant";
        List attendants = getHibernateTemplate().findByNamedParam(query, "attendant", attendant);
        
        return attendants;
    }
    
    public void storeGateway(Gateway gateway) {
        getHibernateTemplate().saveOrUpdate(gateway);
    }

    /**
     * This is for testing only.
     * TODO: need to find a better way of cleaning database between tests
     */
    public void clear() {
        resetToFactoryDefault();       
        List gateways = getHibernateTemplate().loadAll(Gateway.class);        
        getHibernateTemplate().deleteAll(gateways);
        List attendants = getHibernateTemplate().loadAll(AutoAttendant.class);
        getHibernateTemplate().deleteAll(attendants);        
    }

    public boolean deleteGateway(Integer id) {
        Gateway g = getGateway(id);
        getHibernateTemplate().delete(g);
        return true;
    }

    public void deleteGateways(Collection selectedRows) {
        // remove gateways from rules first
        List rules = getRules();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.removeGateways(selectedRows);
            storeRule(rule);
        }
        // remove gateways from the database
        for (Iterator i = selectedRows.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            deleteGateway(id);
        }
    }

    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    public Collection getAvailableGateways(Integer ruleId) {
        DialingRule rule = getRule(ruleId);
        if (null == rule) {
            return Collections.EMPTY_LIST;
        }
        List allGateways = getGateways();
        return rule.getAvailableGateways(allGateways);
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
        Organization organization = m_coreContext.loadRootOrganization();
        try {
            EmergencyRoutingRules rules = new EmergencyRoutingRules();
            rules.generate(m_emergencyRouting, organization.getDnsDomain());
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

    public List getGatewayByIds(Collection gatewayIds) {
        List gateways = new ArrayList(gatewayIds.size());
        for (Iterator i = gatewayIds.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            gateways.add(getGateway(id));
        }
        return gateways;
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
}
