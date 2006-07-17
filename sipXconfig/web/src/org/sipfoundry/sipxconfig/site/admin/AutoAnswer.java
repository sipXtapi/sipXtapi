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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class AutoAnswer extends BasePage {
    public abstract String getGroupsString();
    public abstract void setGroupsString(String groupsString);
    
    public abstract PhoneContext getPhoneContext();
    public abstract SettingDao getSettingDao();
    
    public abstract Collection getGroupsCandidates();
    public abstract void setGroupCandidates(Collection groupsList);
    
    public void buildGroupCandidates(String groupsString) {
        List allGroups = getPhoneContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);        
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        
        if (!cycle.isRewinding()) {
/*            Phone phone = getPhone();        

            if (getGroupsString() == null) {
                List groups = phone.getGroupsAsList();
                if (groups != null && groups.size() > 0) {
                    String groupsString = getSettingDao().getGroupsAsString(groups); 
                    setGroupsString(groupsString);
                }
            }
*/        }
        
        super.renderComponent(writer, cycle);
        
        if (cycle.isRewinding()) {
/*            Phone phone = getPhone();        
            // Don't take any actions if the page is not valid
            if (!TapestryUtils.isValid((AbstractPage) getPage())) {
                return;
            }

            String groupsString = getGroupsString();
            if (groupsString != null) {
                List groups = getSettingDao().getGroupsByString(Phone.GROUP_RESOURCE_ID, groupsString);
                phone.setGroupsAsList(groups);
            }
*/        }
    }
}
