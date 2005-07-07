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
package org.sipfoundry.sipxconfig.setting;

/**
 * Information about how a setting is rendered to user.
 */
public class RenderProperties {
    
    private boolean m_isPasswordField;       

    /** If ui should mark it hidden */
    public boolean isPassword() {
        return m_isPasswordField;
    }
    
    public void setPassword(boolean isPasswordField) {
        m_isPasswordField = isPasswordField;
    }
    
    public static RenderProperties createPasswordField() {
        RenderProperties p = new RenderProperties();
        p.setPassword(true);
        return p;
    }
}
