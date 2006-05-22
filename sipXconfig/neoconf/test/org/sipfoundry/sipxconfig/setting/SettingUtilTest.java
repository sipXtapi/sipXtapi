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
    
//    public void testGetSettingFromNode() {
//        Setting root = TestHelper.loadSettings("../test/org/sipfoundry/sipxconfig/setting/games.xml");
//        Setting cards = root.getSetting("cards");
//        Setting ace = root.getSetting("cards/card/A");
//        String acePath = ace.getPath();
//        assertSame(ace, SettingUtil.getSettingFromNode(cards, acePath));        
//    }

    public void testIsAdvancedIncludingParents() {
        Setting root = loadSettings("advanced.xml");
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced/setting")));
        assertFalse(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("not-advanced/setting")));
        assertFalse(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("not-advanced")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advanced-setting/setting")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advance-and-advanded-setting/setting")));
        assertTrue(SettingUtil.isAdvancedIncludingParents(root, root.getSetting("advance-and-advanded-setting")));
    }    
    
    Setting loadSettings(String resource) {
        InputStream in = getClass().getResourceAsStream(resource);
        return TestHelper.loadSettings(in);
    }
}
