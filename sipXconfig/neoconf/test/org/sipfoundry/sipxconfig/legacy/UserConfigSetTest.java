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
package org.sipfoundry.sipxconfig.legacy;

import java.io.InputStreamReader;
import java.io.Reader;
import java.util.Map;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;

public class UserConfigSetTest extends TestCase {

    private UserConfigSet m_configSet;

    protected void setUp() throws Exception {
        Reader controlXml = new InputStreamReader(getClass().getResourceAsStream(
        "userConfigSet.test.xml"));
        String content = IOUtils.toString(controlXml);
        m_configSet = new UserConfigSet();
        m_configSet.setContent(content);
    }

    public void testGetPermissions() {
        Map permissions = m_configSet.getPermissions();
        assertEquals(Boolean.TRUE, permissions.get(Permission.AUTO_ATTENDANT));
        assertEquals(Boolean.FALSE, permissions.get(Permission.FORWARD_CALLS_EXTERNAL));
        assertEquals(Boolean.TRUE, permissions.get(Permission.INTERNATIONAL_DIALING));
        assertEquals(Boolean.TRUE, permissions.get(Permission.LOCAL_DIALING));
        assertEquals(Boolean.TRUE, permissions.get(Permission.LONG_DISTANCE_DIALING));
        assertEquals(Boolean.TRUE, permissions.get(Permission.VOICEMAIL));        
        assertEquals(Boolean.TRUE, permissions.get(Permission.RESTRICTED_DIALING));
        assertEquals(Boolean.FALSE, permissions.get(Permission.RECORD_SYSTEM_PROMPTS));
        
        assertFalse(permissions.containsKey(Permission.TOLL_FREE_DIALING));
    }
    
    public void testHasPermission() {
        assertTrue(m_configSet.hasPermission(Permission.VOICEMAIL));
        assertFalse(m_configSet.hasPermission(Permission.RECORD_SYSTEM_PROMPTS));
        // the next one test if we return FALSE for properties that are not config set
        assertFalse(m_configSet.hasPermission(Permission.TOLL_FREE_DIALING));
    }
    
    public void testGetClearTextPassword() {
        assertEquals("1234", m_configSet.getClearTextPassword());
    }
    
}
