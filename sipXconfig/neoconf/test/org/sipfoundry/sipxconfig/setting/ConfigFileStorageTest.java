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
package org.sipfoundry.sipxconfig.setting;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class ConfigFileStorageTest extends TestCase {
    static String CONFIG_FILE = "test-config.in";

    private String m_configDirectory;

    private ConfigFileStorage m_storage;

    private SettingImpl m_andorra;

    protected void setUp() throws Exception {
        InputStream configStream = ConfigFileStorageTest.class.getResourceAsStream(CONFIG_FILE);
        m_configDirectory = TestHelper.getTestDirectory();
        TestHelper.copyStreamToDirectory(configStream, m_configDirectory, CONFIG_FILE);

        m_storage = new ConfigFileStorage(m_configDirectory);

        m_andorra = new SettingImpl();
        m_andorra.setProfileName(CONFIG_FILE);
        m_andorra.setName("Andorra");
    }

    public void testGet() {

        assertEquals("83.51", m_storage.getValue(m_andorra));
    }

    public void testPut() throws Exception {
        String newValue = "99.999";
        m_storage.setValue(m_andorra, newValue);
        assertEquals(newValue, m_storage.getValue(m_andorra));

        BufferedReader reader = new BufferedReader(new FileReader(new File(m_configDirectory,
                CONFIG_FILE)));

        m_storage.flush();
        boolean found = false;
        while (reader.ready()) {
            String line = reader.readLine();
            if (line.startsWith(m_andorra.getName())) {
                assertTrue(line.endsWith(newValue));
                found = true;
            }
        }
        assertTrue("Setting not found", found);
    }

    public void testRemove() throws Exception {
        m_storage.remove(m_andorra);
        m_storage.flush();
        BufferedReader reader = new BufferedReader(new FileReader(new File(m_configDirectory,
                CONFIG_FILE)));
        // need to implement remove
        /*
        while (reader.ready()) {
            String line = reader.readLine();
            assertFalse(line.startsWith(m_andorra.getName()));
        }
        */
    }

}
