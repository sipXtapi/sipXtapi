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
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.admin.dialplan.config.EmergencyRoutingRules;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.Organization;

/**
 * DialPlanManager TODO: need interface and hibernate persistence implementation
 */
public class DialPlanManager {
    private List m_gateways = new ArrayList();
    private String m_configDirectory;

    private EmergencyRouting m_emergencyRouting = new EmergencyRouting(); 

    private FlexibleDialPlan m_flexDialPlan = new FlexibleDialPlan();

    private transient ConfigGenerator m_generator;

    private DialingRuleFactory m_ruleFactory;
    
    private CoreContext m_coreContext;

    public List getGateways() {
        return m_gateways;
    }

    public void setGateways(List gateways) {
        m_gateways = gateways;
    }

    private Gateway getOrgGateway(Integer id) {
        if (null == id) {
            return null;
        }
        Object key = new Gateway(id);
        int i = m_gateways.indexOf(key);
        if (i < 0) {
            return null;
        }
        Gateway g = (Gateway) m_gateways.get(i);
        return g;
    }

    public Gateway getGateway(Integer id) {
        Gateway g = getOrgGateway(id);
        return null != g ? (Gateway) g.detach() : null;
    }

    public boolean updateGateway(Integer id, Gateway gatewayData) {
        Gateway gateway = getOrgGateway(id);
        if (null == gateway) {
            return false;
        }
        gateway.update(gatewayData);
        return true;
    }

    public boolean addGateway(Gateway gateway) {
        if (!m_gateways.remove(gateway)) {
            m_gateways.add(gateway);
            return true;
        }
        return false;
    }

    public void clear() {
        m_gateways.clear();
        m_flexDialPlan.getRules().clear();
    }

    public boolean deleteGateway(Integer id) {
        return m_gateways.remove(new Gateway(id));
    }

    public void deleteGateways(Collection selectedRows) {
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
        Set gateways = new HashSet(getGateways());
        Collection ruleGateways = rule.getGateways();
        gateways.removeAll(ruleGateways);
        return gateways;
    }

    public FlexibleDialPlan getFlexDialPlan() {
        return m_flexDialPlan;
    }

    public void setFlexDialPlan(FlexibleDialPlan flexDialPlan) {
        m_flexDialPlan = flexDialPlan;
    }

    public ConfigGenerator generateDialPlan() {
        ConfigGenerator generator = new ConfigGenerator();
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
            gateways.add(getOrgGateway(id));
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
