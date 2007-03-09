/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.InputStream;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class SettingUtilTest extends TestCase {

    public void testFilter() {
        Setting root = loadSettings("simplemodel.xml");
        Collection settings = SettingUtil.filter(SettingFilter.ALL, root);
        // 3 would mean root was included and it's supposed to be non-inclusive
        assertEquals(2, settings.size());
    }

    public void testGetSettingByPath() {
        Setting root = loadSettings("simplemodel.xml");
        Setting setting = root.getSetting("group/setting");
        assertNotNull(setting);
        assertEquals("setting", setting.getName());
    }

    public void testIsLeaf() {
        SettingSet root = new SettingSet();
        assertTrue(SettingUtil.isLeaf(root));
        SettingSet child = new SettingSet("child1");
        root.addSetting(child);
        assertFalse(SettingUtil.isLeaf(root));
        assertTrue(SettingUtil.isLeaf(child));
    }

    public void testIsAdvancedIncludingParents() {
        Setting root = loadSettings("advanced.xml");
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced/setting")));
        assertFalse(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("not-advanced/setting")));
        assertFalse(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("not-advanced")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced-setting/setting")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root
                .getSetting("advance-and-advanded-setting/setting")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root
                .getSetting("advance-and-advanded-setting")));
    }

    Setting loadSettings(String resource) {
        InputStream in = getClass().getResourceAsStream(resource);
        return TestHelper.loadSettings(in);
    }
}
