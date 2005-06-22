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
package org.sipfoundry.sipxconfig.site.setting;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class GroupForm extends BaseComponent {
    
    public abstract Group getGroup();
    
    public abstract SettingDao getSettingContext();
    
    public abstract String getReturnPage();
    
    public abstract String getResource();

    public void apply(IRequestCycle cycle_) {
        save();
    }

    public void ok(IRequestCycle cycle) {
        save();
        cycle.activate(getReturnPage());
    }
    
    void save() {
        Group group = getGroup();
        group.setResource(getResource());        
        getSettingContext().storeGroup(getGroup());
    }

    public void cancel(IRequestCycle cycle) {        
        cycle.activate(getReturnPage());
    }
}
