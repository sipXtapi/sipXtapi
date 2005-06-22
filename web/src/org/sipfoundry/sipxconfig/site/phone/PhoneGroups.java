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
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class PhoneGroups extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "PhoneGroups";
    
    public static final String EDIT_PHONE_GROUP_PAGE = "EditPhoneGroup";
    
    public abstract void setGroups(Collection groups);
    
    public abstract Collection getGroups();
    
    public abstract PhoneContext getPhoneContext();
        
    public void addGroup(IRequestCycle cycle) {
        cycle.activate(EDIT_PHONE_GROUP_PAGE);
    }    
    
    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setGroups(context.getGroups());
    }
}
