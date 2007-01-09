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
        VoicemailManagerImpl mgr = new VoicemailManagerImpl();
        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        mgr.setMailstoreDirectory(thisDir);
        
        assertEquals(0, mgr.getVoicemail("200", "inbox-bogus").size());
        assertEquals(0, mgr.getVoicemail("200-bogus", "inbox").size());
    }

    public void testGetVoicemail() {
        VoicemailManagerImpl mgr = new VoicemailManagerImpl();        
        String thisDir = TestUtil.getTestSourceDirectory(getClass());
        mgr.setMailstoreDirectory(thisDir);
        List<Voicemail> vm = mgr.getVoicemail("200", "inbox");
        assertEquals(1, vm.size());
        assertEquals("00000001-00", vm.get(0).getBasename());
        assertTrue(vm.get(0).getMediaFile().exists());
    }
    
    public void testBasename() {
        assertEquals("bird", VoicemailManagerImpl.basename("bird.wav"));
        assertEquals("bird", VoicemailManagerImpl.basename("bird"));
        assertEquals("bird.species", VoicemailManagerImpl.basename("bird.species.txt"));
    }
}
