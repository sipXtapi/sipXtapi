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
package org.sipfoundry.sipxconfig.common;

/**
 * UserName
 * 
 * Represents SIP usernames for both human users and services like the auto attendant
 */
public class UserName extends BeanWithId implements NamedObject {
    private String m_name;

    /** No-args constructor required for Hibernate */
    public UserName() {}
    
    public UserName(String name) {
        m_name = name;
    }
    
    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public boolean equals(Object o) {
        boolean isEqual = false;
        if (o instanceof UserName) {
            isEqual = m_name.equals(((UserName) o).getName());
        }
        return isEqual;
    }

    public int hashCode() {
        return m_name.hashCode();
    }
}
