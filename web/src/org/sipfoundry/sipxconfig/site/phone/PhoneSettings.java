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

/**
 * Edit vendor specific phone setttings in abstract manor using setting model of meta
 * data
 */
public abstract class PhoneSettings extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "PhoneSettings"; 

    public abstract Integer getEndpointId();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setEndpointId(Integer id);
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public abstract String getParentSettingName();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setParentSettingName(String name); 

    public abstract Setting getParentSetting();
    
    public abstract void setParentSetting(Setting parent);
    
    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setEndpoint(context.loadEndpoint(getEndpointId()));
        Phone phone = context.getPhone(getEndpoint()); 
        Setting root = getEndpoint().getSettings(phone);
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        PhoneContext dao = getPhoneContext();
        dao.storeEndpoint(getEndpoint());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }    
}
