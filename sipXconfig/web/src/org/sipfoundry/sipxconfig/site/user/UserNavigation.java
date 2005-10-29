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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class UserNavigation extends BaseComponent {

    /** REQUIRED PARAMETER */
    public abstract void setUser(User user);
    
    public abstract User getUser();
    
    public abstract void setSettings(Setting settings);
    
    public abstract Setting getSettings();

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();
    
    public abstract CoreManager getCoreContext();

    public void editUser(IRequestCycle cycle) {
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer userId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setUserId(userId);

        cycle.activate(page);
    }
    
    public void editSettings(IRequestCycle cycle) {
        UserSettings page = (UserSettings) cycle.getPage(UserSettings.PAGE);

        Object[] params = cycle.getServiceParameters();
        Integer userId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        page.setUserId(userId);

        String section = (String) TapestryUtils.assertParameter(String.class, params, 1);        
        page.setParentSettingName(section);
        
        cycle.activate(page);        
    }
    
    /**
     * Used for contructing parameters for EditSettings DirectLink
     */
    public Object[] getEditSettingListenerParameters() {
        return new Object[] { 
            getUser().getId(),
            getCurrentSetting().getName() 
        };
    }
    
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        setSettings(getUser().getSettings());
    }  
}
