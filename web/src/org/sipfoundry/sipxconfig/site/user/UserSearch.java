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
package org.sipfoundry.sipxconfig.site.user;

import java.util.Collections;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;


public abstract class UserSearch extends BaseComponent implements PageRenderListener {
    
    public abstract User getUser();

    public abstract void setUser(User user);
    
    public abstract void setUsers(List users);
    
    public abstract List getUsers();
    
    public void search(IRequestCycle cycle) {
        PhoneContext context = PhonePageUtils.getPhoneContext(cycle);
        setUsers(context.loadUserByTemplateUser(getUser()));
    }
    
    public void pageBeginRender(PageEvent event_) {        
        setUser(new User());
        setUsers(Collections.EMPTY_LIST);
    }
}
