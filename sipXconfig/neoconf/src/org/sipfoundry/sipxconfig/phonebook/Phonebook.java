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

import java.util.Collection;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.setting.Group;

public class Phonebook extends BeanWithId {
    private String m_membersCsvFilename;
    private Set<Group> m_members = new TreeSet<Group>();
    
    public String getMembersCsvFilename() {
        return m_membersCsvFilename;
    }
    
    public void setMembersCsvFilename(String externalUsersFilename) {
        m_membersCsvFilename = externalUsersFilename;
    }
    
    public Set<Group> getMembers() {
        return m_members;
    }
    
    public void setMembers(Set<Group> members) {
        m_members = members;
    }
    
    public void replaceMembers(Collection<Group> groups) {
        m_members.clear();
        m_members.addAll(groups);
    }    
}
