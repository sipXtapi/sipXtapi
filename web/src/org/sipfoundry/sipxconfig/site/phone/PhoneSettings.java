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
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Edit vendor specific phone setttings in abstract manor using setting model of meta
 * data
 */
public abstract class PhoneSettings extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "PhoneSettings"; 

    public abstract int getEndpointId();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setEndpointId(int id);
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public abstract String getParentSettingGroupName();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setParentSettingGroupName(String name); 

    public abstract SettingGroup getParentSettingGroup();
    
    public abstract void setParentSettingGroup(SettingGroup parent);
    
    public abstract Setting getCurrentSetting();
    
    public abstract void setCurrentSetting(Setting setting);
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setEndpoint(context.loadEndpoint(getEndpointId()));
        Phone phone = context.getPhone(getEndpoint()); 
        SettingGroup root = getEndpoint().getSettings(phone);
        SettingGroup parent = (SettingGroup) root.getSetting(getParentSettingGroupName());
        setParentSettingGroup(parent);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle);
        dao.storeEndpoint(getEndpoint());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }    
}
