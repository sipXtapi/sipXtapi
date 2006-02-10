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

import java.io.IOException;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;


/**
 * Tapestry Visit object - session parameters for sipXconfig
 */
public class UserSession {

    /**
     * true if we want to display title bar and navigation false for testing and when embedding
     * pages in profilegen
     */
    private boolean m_navigationVisible = true;

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
    
    public boolean isLoggedIn() {
        return getUserId() != null;
    }

    public void login(Integer userId, boolean admin) {
        m_userId = userId;
        m_admin = admin;
    }
    
    public ILink getLogoutLink(IEngineService restartService) {
        return restartService.getLink(false, null);
    }
    
    public void logout(IEngineService restartService, IRequestCycle cycle) {
        try {
            restartService.service(cycle);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }            
    }
}
