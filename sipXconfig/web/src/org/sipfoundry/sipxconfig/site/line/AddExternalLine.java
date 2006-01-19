/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class AddExternalLine extends PageWithCallback implements PageRenderListener {
    
    public static final String PAGE = "AddExternalLine";
    
    public abstract org.sipfoundry.sipxconfig.phone.LineSettings getSettings();
    
    public abstract void setSettings(org.sipfoundry.sipxconfig.phone.LineSettings settings);
    
    public abstract void setPhoneId(Integer phoneId);
    
    public abstract Integer getPhoneId();
    
    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract void setPhoneContext(PhoneContext context);
    
    public abstract PhoneContext getPhoneContext();
    
    public void ok(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            getPhoneContext().storePhone(getPhone());
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Phone phone = getPhone();
        if (phone == null) {
            phone = getPhoneContext().loadPhone(getPhoneId());
            setPhone(phone);
            Line line = phone.createLine();
            phone.addLine(line);
            org.sipfoundry.sipxconfig.phone.LineSettings settings = 
                (org.sipfoundry.sipxconfig.phone.LineSettings) line.getAdapter(
                        org.sipfoundry.sipxconfig.phone.LineSettings.class);
            setSettings(settings);
        }
    }
}
