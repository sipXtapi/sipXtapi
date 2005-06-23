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
 * UserGroup - class representing legacy user group
 * We will not attempt to create optimal Hibernate mapping for this object.
 * Just enough to make possible database querying.
 */
public class UserGroup extends BeanWithId {
    private String m_name;
    private Integer m_parentGroupId;    
    private Integer m_rcsId;
    
    public String getName() {
        return m_name;
    }
    
    public void setName(String name) {
        m_name = name;
    }
    
    public Integer getParentGroupId() {
        return m_parentGroupId;
    }
    
    public void setParentGroupId(Integer parent) {
        m_parentGroupId = parent;
    }
    
    public Integer getRcsId() {
        return m_rcsId;
    }
    
    public void setRcsId(Integer rcsId) {
        m_rcsId = rcsId;
    }
}
