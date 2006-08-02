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
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class AddExternalLine extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "AddExternalLine";

    public abstract LineInfo getLineInfo();

    public abstract void setPhoneId(Integer phoneId);

    public abstract Integer getPhoneId();

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    public abstract PhoneContext getPhoneContext();

    public void ok() {
        if (TapestryUtils.isValid(this)) {
            Phone phone = getPhone();
            Line line = phone.createLine();
            phone.addLine(line);
            line.setLineInfo(getLineInfo());
            getPhoneContext().storePhone(getPhone());
        }
    }

    public void pageBeginRender(PageEvent event_) {
        Phone phone = getPhone();
        if (phone == null) {
            phone = getPhoneContext().loadPhone(getPhoneId());
            setPhone(phone);
        }
    }
}
