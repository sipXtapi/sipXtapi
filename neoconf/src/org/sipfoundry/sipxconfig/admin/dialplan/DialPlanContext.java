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

import org.sipfoundry.sipxconfig.admin.dialplan.config.ConfigGenerator;
import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * DialPlanContext
 */
public interface DialPlanContext extends DataObjectSource  {
    
    public static final String CONTEXT_BEAN_NAME = "dialPlanContext";

    public abstract List getGateways();

    public abstract Gateway getGateway(Integer id);

    public abstract void storeGateway(Gateway gateway);

    public abstract void clear();

    public abstract boolean deleteGateway(Integer id);

    public abstract void deleteGateways(Collection selectedRows);

    /**
     * Returns the list of gateways available for a specific rule
     * 
     * @param ruleId id of the rule for which gateways are checked
     * @return collection of available gateways
     */
    public abstract Collection getAvailableGateways(Integer ruleId);

    public abstract ConfigGenerator generateDialPlan();

    public abstract void activateDialPlan();

    public abstract void applyEmergencyRouting();

    public abstract ConfigGenerator getGenerator();

    public abstract DialingRuleFactory getRuleFactory();

    public abstract List getGatewayByIds(Collection gatewayIds);

    public abstract EmergencyRouting getEmergencyRouting();
    
    public abstract void storeAutoAttendant(AutoAttendant attendant);
    
    public abstract AutoAttendant getOperator();
    
    public abstract AutoAttendant getAutoAttendant(Integer id);
    
    public abstract List getAutoAttendants();
    
    public abstract void deleteAutoAttendantsByIds(Collection attendantsIds);

    public void storeRule(DialingRule rule);

    public List getRules();

    public DialingRule getRule(Integer id);

    public void deleteRules(Collection selectedRows);

    public void duplicateRules(Collection selectedRows);
    
    public void moveRules(Collection selectedRows, int step);

    public List getGenerationRules();
    
    public void resetToFactoryDefault();        
}