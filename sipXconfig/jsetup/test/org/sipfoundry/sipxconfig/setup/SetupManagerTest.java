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
package org.sipfoundry.sipxconfig.setup;

import java.io.File;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.TestUtil;

public class SetupManagerTest extends TestCase {
    
    public void testUpgrade() {
        SetupManager mgr = new SetupManager();
        mgr.setBackupDirectory(TestUtil.getBuildDirectory("jsetup"));
        
        File[] backups = mgr.getBackups();
        assertNotNull(backups);
    }
}
