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
package org.sipfoundry.sipxconfig.core;

public class UserRole {

    private int m_usrsId;

    private String m_rolesName;

    public int getUsrsId() {
        return m_usrsId;
    }

    public void setUsrsId(int usrs_id) {
        this.m_usrsId = usrs_id;
    }

    public String getRolesName() {
        return m_rolesName;
    }

    public void setRolesName(String roles_name) {
        this.m_rolesName = roles_name;
    }

}

