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

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract int getPhoneId();
    
    public abstract void setPhoneId(int id);
    
    public void ok(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle); 
        dao.storeEndpoint(getPhone().getEndpoint());
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle); 
        dao.storeEndpoint(getPhone().getEndpoint());
    }
    
    public void cancel(IRequestCycle cycle) {
        setPhone(null);
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setPhone(context.getPhone(getPhoneId()));
    }       
}
