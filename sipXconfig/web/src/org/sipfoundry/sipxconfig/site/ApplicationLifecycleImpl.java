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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.engine.state.ApplicationStateManager;

/** 
 * Logout copied from Vlib example, also see
 * http://thread.gmane.org/gmane.comp.java.tapestry.user/31641
 */
public class ApplicationLifecycleImpl implements ApplicationLifecycle {
    private boolean m_discardSession;

    private ApplicationStateManager m_stateManager;

    public void setStateManager(ApplicationStateManager stateManager) {
        m_stateManager = stateManager;
    }

    public void logout() {
        m_discardSession = true;
        if (m_stateManager.exists(UserSession.SESSION_NAME)) {
            UserSession userSession = (UserSession) m_stateManager.get(UserSession.SESSION_NAME);
            userSession.logout();
        }
    }

    public boolean getDiscardSession() {
        return m_discardSession;
    }
}
