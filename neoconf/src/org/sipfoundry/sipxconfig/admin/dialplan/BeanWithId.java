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
package org.sipfoundry.sipxconfig.admin.dialplan;

/**
 * ObjectIdentity - this is temporary class to simplify implementation of model layer
 * 
 * Hibernate advises against using object identifiers in equals and hashCode methods
 */
class BeanWithId {
    private static int s_id;

    private Integer m_id;

    BeanWithId() {
        this(new Integer(s_id++));        
    }
    
    BeanWithId(Integer id) {
        setId(id);
    }

    private void setId(Integer id) {
        m_id = id;
    }
    
    public Integer getId() {
        return m_id;
    }

    public boolean equals(Object o) {
        if (!(o instanceof BeanWithId)) {
            return false;
        }
        BeanWithId other = (BeanWithId) o;
        return m_id.equals(other.m_id);
    }
    
    public int hashCode() {
        return m_id.hashCode();
    }
}
