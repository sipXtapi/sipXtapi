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
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;
import org.sipfoundry.sipxconfig.site.phone.PhoneGroups;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

/**
 * TestPage page
 */
public abstract class TestPage extends BasePage {
    public static final String PAGE = "TestPage"; 
    
    public abstract DialPlanContext getDialPlanManager();
    
    public abstract GatewayContext getGatewayContext();    
    
    public abstract PhoneContext getPhoneContext();
    
    public abstract CallGroupContext getCallGroupContext();
    
    public void resetDialPlans(IRequestCycle cycle_) {
        getDialPlanManager().clear();
        getGatewayContext().clear();
    }
    
    public void resetPhoneContext(IRequestCycle cycle_) {
        getPhoneContext().clear();
    }

    public void resetCallGroupContext(IRequestCycle cycle_) {
        getCallGroupContext().clear();
    }
    
    public void newPhoneGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.newGroup("phone", PhoneGroups.PAGE);
        cycle.activate(page);
    }
    
    public void goToRestartReminderPage(IRequestCycle cycle) {
        RestartReminder page = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        page.setNextPage(PAGE);
        cycle.activate(page);
    }
}
