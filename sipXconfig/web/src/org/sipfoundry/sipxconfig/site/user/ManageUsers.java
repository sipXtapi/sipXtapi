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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;


public abstract class ManageUsers extends BasePage 
        implements PageRenderListener {
    
    public static final String PAGE = "ManageUsers";
    private static final String USER_TABLE_COMPONENT_ID = "userTable";

    public abstract Collection getUsers();
    
    public abstract void setUsers(Collection users);

    public abstract CoreContext getCoreContext();
    
    public void addUser(IRequestCycle cycle) {
        NewUser page = (NewUser) cycle.getPage(NewUser.PAGE);
        cycle.activate(page);
    }
    
    public void editUser(IRequestCycle cycle) {
        Integer userId = (Integer) TapestryUtils.assertParameter(Integer.class, 
                cycle.getServiceParameters(), 0);        
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);

        // When we navigate to the Edit User page, clicking OK or Cancel on that
        // page should send the user back here
        ICallback callback = new PageCallback(PAGE);
        page.setCallback(callback);

        cycle.activate(page);        
    }
    
    public void deleteUsers(IRequestCycle cycle_) {
        UserTable table = (UserTable) getComponent(USER_TABLE_COMPONENT_ID);
        SelectMap selections = table.getSelections();
        Collection selected = selections.getAllSelected();
        getCoreContext().deleteUsers(selected);
    }
    
    public void pageBeginRender(PageEvent event_) {
        CoreContext core = getCoreContext();
        setUsers(core.loadUserByTemplateUser(new User()));
    }        
}
