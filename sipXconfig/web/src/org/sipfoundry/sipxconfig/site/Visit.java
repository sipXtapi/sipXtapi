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
    private boolean m_navigationVisible;
    
    public boolean isNavigationVisible() {
        return m_navigationVisible;
    }
    
    public void setNavigationVisible(boolean navigationVisible) {
        m_navigationVisible = navigationVisible;
    }
}
