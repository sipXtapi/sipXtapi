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
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Set;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.setting.Group;

public class Phonebook extends BeanWithId {
    private String m_externalUsersFilename;
    private Set<Group> m_userMembers;
    
    public String getExternalUsersFilename() {
        return m_externalUsersFilename;
    }
    public void setExternalUsersFilename(String externalUsersFilename) {
        m_externalUsersFilename = externalUsersFilename;
    }
    public Set<Group> getUserMembers() {
        return m_userMembers;
    }
    public void setUserMembers(Set<Group> userGroups) {
        m_userMembers = userGroups;
    }
}
