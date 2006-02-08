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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;

/**
 * Tapestry Visit object - session parameters for sipXconfig
 */
public class Visit {
    public static final String BEAN_NAME = "visit";

    /**
     * true if we want to display title bar and navigation false for testing and when embedding
     * pages in profilegen
     */
    private boolean m_navigationVisible = true;

    private boolean m_admin;

    private int m_tablePageSize;

    /**
     * user that is currently logged in
     */
    private Integer m_userId;

    public boolean isNavigationVisible() {
        return m_navigationVisible;
    }

    public void setNavigationVisible(boolean navigationVisible) {
        m_navigationVisible = navigationVisible;
    }

    public int getTablePageSize() {
        return m_tablePageSize;
    }

    public void setTablePageSize(int tablePageSize) {
        m_tablePageSize = tablePageSize;
    }

    public boolean isAdmin() {
        return m_admin;
    }

    public Integer getUserId() {
        return m_userId;
    }

    public void login(Integer userId, boolean admin) {
        m_userId = userId;
        m_admin = admin;
    }

    public void logout(IRequestCycle cycle) {
        // Clear the visit state so we forget about this user and their admin rights, if any
        clear();
        
        // PORT Logout
    }

    void clear() {
        m_userId = null;
        m_admin = false;
    }
}
