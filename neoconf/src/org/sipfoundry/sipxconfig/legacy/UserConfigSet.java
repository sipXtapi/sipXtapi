/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.legacy;

import java.util.Set;

public class UserConfigSet extends ConfigSet {
    private Set m_users;

    public Set getUsers() {
        return m_users;
    }

    public void setUsers(Set users) {
        m_users = users;
    }
}
