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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;


public abstract class AddPhoneUser extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "AddPhoneUser"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    /** REQUIRED PROPERTY */
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public void cancel(IRequestCycle cycle) {
        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        cycle.activate(page);
    }
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setPhone(context.getPhone(getPhoneId()));
    }           
}
