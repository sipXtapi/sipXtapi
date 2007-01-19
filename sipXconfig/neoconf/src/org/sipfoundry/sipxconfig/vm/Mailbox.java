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
package org.sipfoundry.sipxconfig.vm;

import java.io.File;

public class Mailbox {
    private File m_mailstoreDirectory;
    private String m_userId;
    
    public Mailbox(File mailstoreDirectory, String userId) {
        m_mailstoreDirectory = mailstoreDirectory;
        m_userId = userId;
    }

    public File getMailstoreDirectory() {
        return m_mailstoreDirectory;
    }

    public void setMailstoreDirectory(File mailstoreDirectory) {
        m_mailstoreDirectory = mailstoreDirectory;
    }

    public String getUserId() {
        return m_userId;
    }

    public void setUserId(String userId) {
        m_userId = userId;
    }
    
    public File getVoicemailPreferencesFile() {
        return new File(getMailstoreDirectory(), getUserId() + "/mailboxprefs.xml");        
    }
}
