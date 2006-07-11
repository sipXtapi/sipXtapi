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
package org.sipfoundry.sipxconfig.site.user;

import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class SupervisorPermission extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "SupervisorPermission";

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract CoreContext getCoreContext();
    public abstract SettingDao getSettingDao();

    public abstract String getSupervisorForGroupsString();    
    public abstract void setSupervisorForGroupsString(String groups);
    public abstract Collection getGroupsCandidates();
    public abstract void setGroupCandidates(Collection groupsList);
    
    public void buildGroupCanidates(String groupsString) {
        List allGroups = getCoreContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);        
    }
    
    public void pageBeginRender(PageEvent event_) {
        User user = getUser();
        if (user == null) {
            user = getCoreContext().loadUser(getUserId());
            setUser(user);
            
            Set groups = user.getSupervisorForGroups();
            if (groups != null && groups.size() > 0) {
                String groupsString = getSettingDao().getGroupsAsString(groups); 
                setSupervisorForGroupsString(groupsString);
            }            
        }
    }

    public void commit() {
        User user = getUser();
        
        String groupsString = getSupervisorForGroupsString();
        if (groupsString != null) {
            List groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID, groupsString);
            for (Group group : (Collection<Group>) groups) {
                user.addSupervisorForGroup(group);
            }
        }

        getCoreContext().saveUser(user);        
    }
}
