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

import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class SettingUtilTest extends TestCase {

    public void testFilter() {
        Setting root = TestHelper.loadSettings("../test/org/sipfoundry/sipxconfig/setting/simplemodel.xml");
        Collection settings = SettingUtil.filter(SettingFilter.ALL, root);
        // 3 would mean root was included and it's supposed to be non-inclusive
        assertEquals(2, settings.size());
    }

    public void testGetSettingFromRoot() {
        Setting root = TestHelper.loadSettings("../test/org/sipfoundry/sipxconfig/setting/simplemodel.xml");
        Setting setting = SettingUtil.getSettingFromRoot(root, "/group/setting");
        assertNotNull(setting);
        assertEquals("setting", setting.getName());
    }

    public void testGetSettingByPath() {
        Setting root = TestHelper.loadSettings("../test/org/sipfoundry/sipxconfig/setting/simplemodel.xml");
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

    public void testZeroSubpath() {
        assertEquals(null, SettingUtil.subpath(null, 1));
        assertEquals("a", SettingUtil.subpath("a", 0));
        assertEquals("a/b/c", SettingUtil.subpath("a/b/c", 0));
    }
        
    public void testOneSubpath() {
        assertEquals("b", SettingUtil.subpath("a/b", 1));
        assertEquals("b", SettingUtil.subpath("/b", 1));
        assertEquals("", SettingUtil.subpath("a/", 1));
    }
    
    public void testNSubpath() {
        assertEquals("c", SettingUtil.subpath("a/b/c", 2));
    }

}
