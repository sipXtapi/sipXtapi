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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public abstract class PhoneDefaults extends BasePage implements PageRenderListener  {
    
    public abstract SettingGroup getCurrentSetting();
    
    public abstract void setCurrentSetting(SettingGroup group);
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);

    public Collection getPhoneSettings() {
        return getPhone().getSettingModel(getEndpoint()).getValues();
    }
    
    public Collection getLineSettings() {
        Line line = (Line) getEndpoint().getLines().get(0);
        return getPhone().getSettingModel(line).getValues();        
    }
    
    public void editSettings(IRequestCycle cycle_) {        
    }
    
    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);        
    }
    
    public void apply(IRequestCycle cycle) {
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        phoneContext.storeEndpoint(getEndpoint());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
    }
}
