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

import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public abstract class PhoneSettings extends BasePage implements PageRenderListener {

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);

    public abstract SettingSet getSettingSet();
    
    public abstract void setSettingSet(SettingSet set);
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        Phone phone = context.getPhone(getPhoneId()); 
        setPhone(phone);
        Endpoint endpoint = phone.getEndpoint();
        setSettingSet(endpoint.getSettings());
    }
}
