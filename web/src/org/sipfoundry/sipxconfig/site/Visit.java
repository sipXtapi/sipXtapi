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


/**
 * Tapestry Visit object - session parameters for sipXconfig
 */
public class Visit {
    /**
     * true if we want to display title bar and navigation false for testing and when embedding
     * pages in profilegen
     */
    private boolean m_navigationVisible;
    
    private boolean m_admin;

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
    
    public void logout() {
        m_userId = null;
    }
}
