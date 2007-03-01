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

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class MailboxManagerTest extends TestCase {
    private MailboxManagerImpl m_mgr;
    
    protected void setUp() {
        m_mgr = new MailboxManagerImpl();
        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        m_mgr.setMailstoreDirectory(thisDir);        
    }
    
    public void testEnabled() {
        assertTrue(m_mgr.isEnabled());
        m_mgr.setMailstoreDirectory("bogus-mogus");
        assertFalse(m_mgr.isEnabled());
    }

    public void testGetVoicemailWhenInvalid() {        
        MailboxManagerImpl mgr = new MailboxManagerImpl();
        Mailbox mbox = mgr.getMailbox("200");
        try {
            mgr.getVoicemail(mbox, "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }


        try {
            mgr.setMailstoreDirectory("bogus");
            mgr.getVoicemail(mbox, "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }
    }
    
    public void testGetVoicemailWhenEmpty() {        
        assertEquals(0, m_mgr.getVoicemail(m_mgr.getMailbox("200"), "inbox-bogus").size());
        assertEquals(0, m_mgr.getVoicemail(m_mgr.getMailbox("200-bogus"), "inbox").size());
    }

    public void testGetInboxVoicemail() {
        List<Voicemail> vm = m_mgr.getVoicemail(m_mgr.getMailbox("200"), "inbox");
        assertEquals(2, vm.size());
        assertEquals("00000001", vm.get(0).getMessageId());
        assertTrue(vm.get(0).getMediaFile().exists());
    }
    
    public void testBasename() {
        assertEquals("bird", MailboxManagerImpl.basename("bird-00.xml"));
        assertEquals("bird", MailboxManagerImpl.basename("bird"));
    }
    
    public void testGetFolders() {
        List<String> folderIds = m_mgr.getMailbox("200").getFolderIds();
        assertEquals(3, folderIds.size());
    }
    
    public void testGetDeletedVoicemail() {
        List<Voicemail> deleted = m_mgr.getVoicemail(m_mgr.getMailbox("200"), "deleted");
        assertEquals(1, deleted.size());
        assertEquals("00000002", deleted.get(0).getMessageId());
    }
    
    public void testLoadPreferencesWhenEmpty() {
        Mailbox mailbox = m_mgr.getMailbox("300");
        MailboxPreferencesReader reader = new MailboxPreferencesReader();
        m_mgr.setMailboxPreferencesReader(reader);
        MailboxPreferences preferences = m_mgr.loadMailboxPreferences(mailbox);
        assertNotNull(preferences);
    }
    
    public void testSavePreferencesWhenEmpty() {
        m_mgr.setMailstoreDirectory(TestHelper.getTestDirectory());
        MailboxPreferencesWriter writer = new MailboxPreferencesWriter();
        writer.setVelocityEngine(TestHelper.getVelocityEngine());
        m_mgr.setMailboxPreferencesWriter(writer);
        Mailbox mailbox = m_mgr.getMailbox("save-prefs-" + System.currentTimeMillis());
        m_mgr.saveMailboxPreferences(mailbox, new MailboxPreferences());
    }

    public void testSavePreferencesWhenNullPreferences() {
        m_mgr.setMailstoreDirectory(TestHelper.getTestDirectory());
        MailboxPreferencesWriter writer = new MailboxPreferencesWriter();
        writer.setVelocityEngine(TestHelper.getVelocityEngine());
        m_mgr.setMailboxPreferencesWriter(writer);
        Mailbox mailbox = m_mgr.getMailbox("save-prefs-" + System.currentTimeMillis());
        m_mgr.saveMailboxPreferences(mailbox, null);
    }
}
