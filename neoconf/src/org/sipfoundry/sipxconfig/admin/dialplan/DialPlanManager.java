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
import org.sipfoundry.sipxconfig.common.Organization;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * DialPlanManager is an implementation of DialPlanContext with hibernate support.
 */
class DialPlanManager extends HibernateDaoSupport implements DialPlanContext {
    private String m_configDirectory;

    private EmergencyRouting m_emergencyRouting = new EmergencyRouting(); 

    private FlexibleDialPlanContext m_flexDialPlan = new FlexibleDialPlan();

    private transient ConfigGenerator m_generator;

    private DialingRuleFactory m_ruleFactory;
    
    private CoreContext m_coreContext;

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

    public List getAutoAttendants() {
        List gateways = getHibernateTemplate().loadAll(AutoAttendant.class);
        return gateways;
    }

    public AutoAttendant getAutoAttendant(Integer id) {
        return (AutoAttendant) getHibernateTemplate().load(AutoAttendant.class, id);
        
    }

    public void storeGateway(Gateway gateway) {
        getHibernateTemplate().saveOrUpdate(gateway);
    }

    /**
     * This is for testing only.
     * TODO: need to find a better way of cleaning database between tests
     */
    public void clear() {
        m_flexDialPlan.resetToFactoryDefault();       
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
        FlexibleDialPlanContext flexDialPlan = getFlexDialPlan();
        List rules = flexDialPlan.getRules();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            DialingRule rule = (DialingRule) i.next();
            rule.removeGateways(selectedRows);
            flexDialPlan.storeRule(rule);
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
        DialingRule rule = m_flexDialPlan.getRule(ruleId);
        if (null == rule) {
            return Collections.EMPTY_LIST;
        }
        List allGateways = getGateways();
        return rule.getAvailableGateways(allGateways);
    }

    public FlexibleDialPlanContext getFlexDialPlan() {
        return m_flexDialPlan;
    }

    public void setFlexDialPlan(FlexibleDialPlanContext flexDialPlan) {
        m_flexDialPlan = flexDialPlan;
    }

    public ConfigGenerator generateDialPlan() {
        ConfigGenerator generator = new ConfigGenerator();
        generator.generate(m_emergencyRouting);
        generator.generate(m_flexDialPlan);
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
}
