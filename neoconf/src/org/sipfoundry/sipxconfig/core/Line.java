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

/**
 * Phone Line controls what sip addresses a logical phone will use
 */
public class Line {
    
    private int m_id;
    
    private String m_name;
    
    private User m_user;

    /**
     * @return Returns the userId.
     */
    public User getUser() {
        return m_user;
    }
    /**
     * @param userId The userId to set.
     */
    public void setUser(User user) {
        m_user = user;
    }
    /**
     * @return Returns the id.
     */
    public int getId() {
        return m_id;
    }
    /**
     * @param id The id to set.
     */
    public void setId(int id) {
        m_id = id;
    }
    /**
     * @return Returns the name.
     */
    public String getName() {
        return m_name;
    }
    /**
     * @param name The name to set.
     */
    public void setName(String name) {
        m_name = name;
    }

}
