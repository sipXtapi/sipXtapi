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
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.User;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;


public abstract class UserSearch extends BaseComponent implements PageRenderListener {
    
    private List m_users = new ArrayList();
    
    public abstract User getUser();

    public abstract void setUser(User user);
        
    public List getUsers() {
        return m_users;
    }
    
    public void setUsers(List users) {
        // keep original collection, reference has already been given to other
        // components.  
        m_users.clear();
        m_users.addAll(users);
    }
    
    public void search(IRequestCycle cycle) {
        PhoneContext context = PhonePageUtils.getPhoneContext(cycle);
        setUsers(context.loadUserByTemplateUser(getUser()));
    }
    
    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {        
            setUser(new User());
        }
    }
    
    protected void cleanupAfterRender(IRequestCycle cycle) {
        super.cleanupAfterRender(cycle);

        // reset my own variables
        m_users.clear();
    }
}
