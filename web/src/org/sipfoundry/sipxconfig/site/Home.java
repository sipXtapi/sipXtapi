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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

/**
 * Home page
 */
public abstract class Home extends BasePage {
    
    public abstract DialPlanContext getDialPlanManager();
    
    public abstract PhoneContext getPhoneContext();
    
    public void resetDialPlans(IRequestCycle cycle_) {
        getDialPlanManager().clear();
    }
    
    public void resetPhoneContext(IRequestCycle cycle_) {
        getPhoneContext().clear();
    }
}
