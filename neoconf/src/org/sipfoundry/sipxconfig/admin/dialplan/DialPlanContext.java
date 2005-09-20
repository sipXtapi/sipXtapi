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
public interface DialPlanContext extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "dialPlanContext";

    public abstract void clear();

    public abstract ConfigGenerator generateDialPlan();

    public abstract void activateDialPlan();

    public abstract void applyEmergencyRouting();

    public abstract ConfigGenerator getGenerator();

    public abstract DialingRuleFactory getRuleFactory();

    public abstract EmergencyRouting getEmergencyRouting();
    
    public abstract void storeAutoAttendant(AutoAttendant attendant);
    
    public abstract void deleteAutoAttendant(AutoAttendant attendant, String scriptsDir);
    
    public abstract AutoAttendant getOperator();
    
    public abstract AutoAttendant getAutoAttendant(Integer id);
    
    public abstract List getAutoAttendants();
    
    public abstract void deleteAutoAttendantsByIds(Collection attendantsIds, String scriptsDir);

    public void storeRule(DialingRule rule);

    public List getRules();

    public DialingRule getRule(Integer id);

    public void deleteRules(Collection selectedRows);

    public void duplicateRules(Collection selectedRows);
    
    public void moveRules(Collection selectedRows, int step);

    public List getGenerationRules();
    
    public void resetToFactoryDefault();
    
    public boolean isDialPlanEmpty();
    
    public String getVoiceMail();
}
