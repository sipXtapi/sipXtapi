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
package org.sipfoundry.sipxconfig.admin.dialplan;

public class VxmlService {
    
    private String m_promptsDirectory;
    
    private String m_scriptsDirectory;

    public String getScriptsDirectory() {
        return m_scriptsDirectory;
    }

    public void setScriptsDirectory(String vxmlScriptsDirectory) {
        m_scriptsDirectory = vxmlScriptsDirectory;
    }

    public String getPromptsDirectory() {
        return m_promptsDirectory;
    }   

    public void setPromptsDirectory(String vxmlPromptsDirectory) {
        m_promptsDirectory = vxmlPromptsDirectory;
    }    
}
