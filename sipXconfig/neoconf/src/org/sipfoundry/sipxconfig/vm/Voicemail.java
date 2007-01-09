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

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class Voicemail implements Comparable, PrimaryKeySource {
    public static final char SEPARATOR = '/';
    private String m_basename;
    private File m_mailbox;
        
    public Voicemail(File mailstoreDirectory, String userId, String folderId, String basename) {
        File userDir = new File(mailstoreDirectory, userId);
        m_mailbox = new File(userDir, folderId);
        m_basename = basename;
    }
    
    public String getFolderId() {
        return m_mailbox.getName();
    }
    
    public String getUserId() {
        return m_mailbox.getParentFile().getName();
    }

    public String getBasename() {
        return m_basename;        
    }
    
    public File getMediaFile() {
        return new File(m_mailbox, m_basename + ".wav");
    }

    public int compareTo(Object o) {
        if (o == null || o instanceof Voicemail) {
            return -1;
        }
        return m_basename.compareTo(((Voicemail) o).getBasename());
    }

    public Object getPrimaryKey() {
        return getUserId() + SEPARATOR + getFolderId() + SEPARATOR + getBasename();
    }
    
    public static String[] decodePrimaryKey(Object primaryKey) {
        String[] ids = primaryKey.toString().split(String.valueOf(SEPARATOR));
        return ids;
    }
}
