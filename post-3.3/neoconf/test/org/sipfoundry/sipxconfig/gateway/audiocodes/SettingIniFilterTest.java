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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.StringWriter;

import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

import junit.framework.TestCase;

public class SettingIniFilterTest extends TestCase {

    private SettingSet m_root;

    protected void setUp() throws Exception {
        m_root = new SettingSet();
        SettingSet group1 = new SettingSet("group1");
        group1.setProfileName("group 1");
        SettingSet group2 = new SettingSet("group2");
        m_root.addSetting(group1);
        m_root.addSetting(group2);

        SettingImpl setting1 = new SettingImpl("xxx1");
        setting1.setProfileName("xxx 1");
        setting1.setValue("v1");
        group1.addSetting(setting1);
        group2.addSetting(setting1);

        SettingImpl setting2 = new SettingImpl("x2");
        setting2.setValue("v2");
        group1.addSetting(setting2);

        group1.addSetting(setting1);
        group1.addSetting(setting2);
    }

    public void testWriteString() throws Exception {
        StringWriter writer = new StringWriter();
        SettingIniFilter filter = new SettingIniFilter(writer, m_root);
        filter.write("kuku");
        assertEquals("kuku", writer.toString());
    }

    public void testWriteSetting() throws Exception {
        StringWriter writer = new StringWriter();
        SettingIniFilter filter = new SettingIniFilter(writer, m_root);
        filter.write("[group 1]");
        filter.write("\n");
        filter.write("abc = cde");
        filter.write("\n");

        String[] lines = writer.toString().split("\n");
        assertEquals("abc = cde", lines[lines.length - 1]);

        filter.write("x2 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("x2 = v2", lines[lines.length - 1]);

        // line starts with spaces
        filter.write("  x2 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("x2 = v2", lines[lines.length - 1]);

        // comment
        filter.write(";  x2 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals(";  x2 = 3", lines[lines.length - 1]);
    }

    public void testWriteSettingOtherGroup() throws Exception {
        StringWriter writer = new StringWriter();
        SettingIniFilter filter = new SettingIniFilter(writer, m_root);
        filter.write("[group2]");
        filter.write("\n");
        filter.write("abc = cde");
        filter.write("\n");

        String[] lines = writer.toString().split("\n");
        assertEquals("abc = cde", lines[lines.length - 1]);

        filter.write("x2 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("x2 = 3", lines[lines.length - 1]);

        filter.write("xxx 1 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("xxx 1 = v1", lines[lines.length - 1]);
    }

    public void testWriteSettingProfileName() throws Exception {
        StringWriter writer = new StringWriter();
        SettingIniFilter filter = new SettingIniFilter(writer, m_root);
        filter.write("[group2]");
        filter.write("\n");
        filter.write("abc = cde");
        filter.write("\n");

        String[] lines = writer.toString().split("\n");
        assertEquals("abc = cde", lines[lines.length - 1]);

        filter.write("xxx 1 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("xxx 1 = v1", lines[lines.length - 1]);

        filter.write("xxx1 = 3");
        filter.write("\n");
        lines = writer.toString().split("\n");
        assertEquals("xxx1 = 3", lines[lines.length - 1]);
    }
}
