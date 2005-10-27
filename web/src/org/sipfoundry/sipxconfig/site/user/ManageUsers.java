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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;


public abstract class ManageUsers extends BasePage {
    
    public static final String PAGE = "ManageUsers";
    private static final String USER_TABLE_COMPONENT_ID = "userTable";

    public abstract CoreContext getCoreContext();

    public abstract Integer getGroupId();   
    public abstract void setGroupId(Integer groupId);
    
    public abstract String getSearchString();
    public abstract void setSearchString(String searchString);
    
    public void searchUsers(IRequestCycle cycle_) {
        // do nothing, page should refresh when rendering        
    }
    
    public void clearSearch(IRequestCycle cycle_) {
        setSearchString(null);
    }
    
    public void addUser(IRequestCycle cycle) {
        // Prep the NewUser page with a null userId so a new user will be created
        NewUser page = (NewUser) cycle.getPage(NewUser.PAGE);
        page.setUserId(null);

        // Go to the New User page
        page.activatePageWithCallback(PAGE, cycle);
    }
    
    public void editUser(IRequestCycle cycle) {
        // Prep the EditUser page with the userId
        Integer userId = (Integer) TapestryUtils.assertParameter(Integer.class, 
                cycle.getServiceParameters(), 0);        
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);

        // Go to the Edit User page
        page.activatePageWithCallback(PAGE, cycle);
    }
    
    public void deleteUsers(IRequestCycle cycle_) {
        UserTable table = (UserTable) getComponent(USER_TABLE_COMPONENT_ID);
        SelectMap selections = table.getSelections();
        Collection selected = selections.getAllSelected();
        getCoreContext().deleteUsers(selected);
    }
}
