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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class AddExternalLine extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "AddExternalLine";

    public abstract org.sipfoundry.sipxconfig.phone.LineSettings getSettings();

    public abstract void setSettings(org.sipfoundry.sipxconfig.phone.LineSettings settings);

    public abstract void setPhoneId(Integer phoneId);

    public abstract Integer getPhoneId();

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    public abstract PhoneContext getPhoneContext();

    public void ok() {
        if (TapestryUtils.isValid(this)) {
            
            // XCF-914 - At least polycom need authid as well as the userid 
            // set when line is registering with a proxy in a domain other that 
            // the outbound proxy.
            org.sipfoundry.sipxconfig.phone.LineSettings settings = getSettings();
            String userId = settings.getUserId();
            settings.setAuthorizationId(userId);            
            
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
            org.sipfoundry.sipxconfig.phone.LineSettings settings = (org.sipfoundry.sipxconfig.phone.LineSettings) line
                    .getAdapter(org.sipfoundry.sipxconfig.phone.LineSettings.class);
            setSettings(settings);
        }
    }
}
