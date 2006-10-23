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
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.File;
import java.util.Collection;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;

public abstract class EditPhonebook extends PageWithCallback implements PageBeginRenderListener {
    public abstract PhonebookManager getPhonebookManager();
    public abstract SettingDao getSettingDao();
    public abstract CoreContext getCoreContext();
    public abstract Phonebook getPhonebook();
    public abstract void setPhonebook(Phonebook phonebook);
    public abstract String getMemberGroupsString();
    public abstract void setMemberGroupsString(String groups);
    public abstract Collection getGroupCandidates();
    public abstract void setGroupCandidates(Collection groupsList);
    
    public void savePhonebook() {
        Phonebook phonebook = getPhonebook();

        String groupsString = getMemberGroupsString();
        if (groupsString != null) {
            List<Group> groups = getSettingDao().getGroupsByString(User.GROUP_RESOURCE_ID,
                    groupsString, true);
            for (Group group : groups) {
                phonebook.addMemberGroup(group);
            }
        }

        getPhonebookManager().savePhonebook(phonebook);
    }
    
    public File getPhonebookFile() {
        String assetFilename = getPhonebook().getExternalUsersFilename();
        if (assetFilename == null) {
            return null;
        }
        return new File(getPhonebookDirectory(), assetFilename);
    }
    
    public void setPhonebookFile(File phonebook) {
        getPhonebook().setExternalUsersFilename(phonebook.getName());
    }
    
    public File getPhonebookDirectory() {
        File d = new File(getPhonebookManager().getExternalUsersDirectory());
        return d;
    }
    
    public void buildGroupCandidates(String groupsString) {
        List allGroups = getCoreContext().getGroups();
        Collection candidates = TapestryUtils.getAutoCompleteCandidates(allGroups, groupsString);
        setGroupCandidates(candidates);
    }

    public void pageBeginRender(PageEvent arg0) {
        Phonebook phonebook = getPhonebook();
        if (phonebook == null) {
            phonebook = getPhonebookManager().getGlobalPhonebook();
            if (phonebook == null) {
                phonebook = new Phonebook();
            }
            setPhonebook(phonebook);            

            Set<Group> groups = phonebook.getUserMembers();
            String groupsString = BeanWithGroups.getGroupsAsString(groups);
            setMemberGroupsString(groupsString);                
        }
    }
}
