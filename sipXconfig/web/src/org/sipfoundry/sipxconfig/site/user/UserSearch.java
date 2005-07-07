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

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;


public abstract class UserSearch extends BaseComponent implements PageRenderListener {
    
    public static final String COMPONENT = "UserSearch";
    
    public abstract User getUser();

    public abstract void setUser(User user);
        
    public abstract List getUsers();
    
    public abstract void setUsers(List users);
    
    public abstract CoreContext getCoreContext();
    
    public void search(IRequestCycle cycle_) {
        // keep original collection, reference has already been given to other
        // components.  
        getUsers().clear();        
        List results = getCoreContext().loadUserByTemplateUser(getUser());
        getUsers().addAll(results);
    }
    
    public void pageBeginRender(PageEvent event_) {
        if (getUser() == null) {        
            setUser(new User());
        }
        if (getUsers() == null) {
            setUsers(new ArrayList());
        }
    }
}
