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

public interface FlexibleDialPlanContext {
    public static final String CONTEXT_BEAN_NAME = "flexDialPlan";

    public void storeRule(DialingRule rule);

    public List getRules();

    public DialingRule getRule(Integer id);

    public void deleteRules(Collection selectedRows);

    public void duplicateRules(Collection selectedRows);
    
    public void moveRules(Collection selectedRows, int step);

    public List getGenerationRules();
    
    public void resetToFactoryDefault();    
}