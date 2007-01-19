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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.test.TestUtil;

public class MailboxTest extends TestCase {
    
    public void testFilePreferencesFile() {
        Mailbox mbox = new Mailbox(new File(TestUtil.getTestSourceDirectory(this.getClass())), "200");
        assertTrue(mbox.getVoicemailPreferencesFile().isFile());        
    }
}
