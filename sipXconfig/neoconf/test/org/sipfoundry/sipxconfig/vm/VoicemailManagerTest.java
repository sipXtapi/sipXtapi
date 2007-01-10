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

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class VoicemailManagerTest extends TestCase {
    private VoicemailManagerImpl m_mgr;
    
    protected void setUp() {
        m_mgr = new VoicemailManagerImpl();        
        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        m_mgr.setMailstoreDirectory(thisDir);        
    }

    public void testGetVoicemailWhenInvalid() {        
        VoicemailManagerImpl mgr = new VoicemailManagerImpl();
        try {
            mgr.getVoicemail("200", "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }


        try {
            mgr.setMailstoreDirectory("bogus");
            mgr.getVoicemail("200", "inbox").size();
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }
    }
    
    public void testGetVoicemailWhenEmpty() {        
        assertEquals(0, m_mgr.getVoicemail("200", "inbox-bogus").size());
        assertEquals(0, m_mgr.getVoicemail("200-bogus", "inbox").size());
    }

    public void testGetInboxVoicemail() {
        List<Voicemail> vm = m_mgr.getVoicemail("200", "inbox");
        assertEquals(1, vm.size());
        assertEquals("00000001-00", vm.get(0).getBasename());
        assertTrue(vm.get(0).getMediaFile().exists());
    }
    
    public void testBasename() {
        assertEquals("bird", VoicemailManagerImpl.basename("bird.wav"));
        assertEquals("bird", VoicemailManagerImpl.basename("bird"));
        assertEquals("bird.species", VoicemailManagerImpl.basename("bird.species.txt"));
    }
    
    public void testGetFolders() {
        List<String> folderIds = m_mgr.getFolderIds("200");
        assertEquals(3, folderIds.size());
    }
    
    public void testGetDeletedVoicemail() {
        List<Voicemail> deleted = m_mgr.getVoicemail("200", "deleted");
        assertEquals(1, deleted.size());
        assertEquals("00000002-00", deleted.get(0).getBasename());
    }
}
