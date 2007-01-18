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
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.io.filefilter.SuffixFileFilter;
import org.sipfoundry.sipxconfig.common.UserException;

public class MailboxManagerImpl implements MailboxManager {    
    private static final FilenameFilter WAV_FILES = new SuffixFileFilter(".wav");
    private File m_mailstoreDirectory;
    private MailboxPreferencesReader m_mailboxPreferencesReader;
    private MailboxPreferencesWriter m_mailboxPreferencesWriter;
   
    public List<String> getFolderIds(String userId) {
        return Arrays.asList(new String[] {"inbox", "deleted", "saved"});
    }
    
    public List<Voicemail> getVoicemail(String userid, String folder) {
        checkMailstoreDirectory();
        File vmdir = new File(new File(m_mailstoreDirectory, userid), folder);
        String[] wavs = vmdir.list(WAV_FILES);
        if (wavs == null) {
            return Collections.emptyList();
        }
        List<Voicemail> vms = new ArrayList(wavs.length);
        for (String wav : wavs) {
            String basename = basename(wav);
            vms.add(new Voicemail(m_mailstoreDirectory, userid, folder, basename));
        }
        return vms;
    }
    
    /**
     * Because in HA systems, admin may change mailstore directory, validate it
     */
    void checkMailstoreDirectory() {
        if (m_mailstoreDirectory == null) {
            throw new MailstoreMisconfigured(null);
        }
        if (!m_mailstoreDirectory.exists()) {
            throw new MailstoreMisconfigured(m_mailstoreDirectory.getAbsolutePath());
        }        
    }
    
    static class MailstoreMisconfigured extends UserException {
        MailstoreMisconfigured() {
            super("Mailstore directory configuration setting is missing.");
        }
        MailstoreMisconfigured(String dir) {
            super(String.format("Mailstore directory does not exist '%s'", dir));
        }
        MailstoreMisconfigured(String message, IOException cause) {
            super(message, cause);
        }
    }
    
    /** 
     * extract file name w/o ext.
     */
    static String basename(String filename) {
        int dot = filename.lastIndexOf('.');
        return dot >= 0 ? filename.substring(0, dot) : filename; 
    }

    public String getMailstoreDirectory() {
        return m_mailstoreDirectory.getPath();
    }

    public void setMailstoreDirectory(String mailstoreDirectory) {
        m_mailstoreDirectory = new File(mailstoreDirectory);
    }
    
    public Mailbox getMailbox(String userId) {
        return new Mailbox(m_mailstoreDirectory, userId);
    }
    
    public void saveMailboxPreferences(Mailbox mailbox, MailboxPreferences preferences) {
        Writer iowriter = null;
        try {
            iowriter = new FileWriter(mailbox.getVoicemailPreferencesFile());
            m_mailboxPreferencesWriter.writeObject(preferences, iowriter);
        } catch (IOException e) {
            throw new MailstoreMisconfigured("Cannot write to mailbox preferences file", e);
        } finally {
            IOUtils.closeQuietly(iowriter);
        }
    }
    
    public MailboxPreferences loadMailboxPreferences(Mailbox mailbox) {
        Reader ioreader = null;
        try {
            ioreader = new FileReader(mailbox.getVoicemailPreferencesFile());
            return m_mailboxPreferencesReader.readObject(ioreader);
        } catch (IOException e) {
            throw new MailstoreMisconfigured("Cannot read from mailbox preferences file", e);
        } finally {
            IOUtils.closeQuietly(ioreader);
        }        
    }
    
    public static class YesNo {
        public String encode(Object o) {
            return Boolean.TRUE.equals(o) ? "yes" : "no";
        }        
    }

    public MailboxPreferencesReader getMailboxPreferencesReader() {
        return m_mailboxPreferencesReader;
    }

    public void setMailboxPreferencesReader(MailboxPreferencesReader mailboxReader) {
        m_mailboxPreferencesReader = mailboxReader;
    }

    public MailboxPreferencesWriter getMailboxPreferencesWriter() {
        return m_mailboxPreferencesWriter;
    }

    public void setMailboxPreferencesWriter(MailboxPreferencesWriter mailboxWriter) {
        m_mailboxPreferencesWriter = mailboxWriter;
    }
}
