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

import junit.framework.TestCase;

public class SettingTest extends TestCase {
    
    private SettingGroup m_root;
    
    private Setting m_apple;
    
    private Setting m_fruit;

    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        m_fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = m_fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingGroup("vegatables"));
    }

    public void testDefaultValue() {
        seedSimpleSettingGroup();
        assertEquals("/fruit/apple", m_apple.getPath());        
        m_apple.setValue("granny smith");
        assertEquals("granny smith", m_apple.getValue());
        assertEquals("granny smith", m_apple.getDefaultValue());
    }    
    
    public void testSetValue() {
        seedSimpleSettingGroup();
        assertNull(m_apple.getValue());
        
        Folder meta = new Folder();
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());

        Setting copy = meta.decorate(m_root);
        Setting m_appleCopy = copy.getSetting("fruit").getSetting("apple");
        m_appleCopy.setValue("macintosh");
        assertNull(m_apple.getValue());
        assertEquals("macintosh", m_appleCopy.getValue());
        assertEquals(null, m_apple.getDefaultValue());
        
        assertNotNull(meta.get(m_apple.getPath()));
        assertEquals(1, meta.size());
        
        // if matches default, should clear value
        m_appleCopy.setValue(null);
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());        
    }
    
    public void testSetHidden() {
        seedSimpleSettingGroup();
        assertFalse(m_apple.isHidden());
        
        Folder meta = new Folder();
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());

        Setting copy = meta.decorate(m_root);
        Setting m_appleCopy = copy.getSetting("fruit").getSetting("apple");
        m_appleCopy.setHidden(true);
        assertTrue(m_appleCopy.isHidden());
        
        assertNotNull(meta.get(m_apple.getPath()));
        assertEquals(1, meta.size());
        
        // if matches default, should clear value
        m_appleCopy.setHidden(false);
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());        
    }
    
    /**
     * If both are set, will it know to keep meta, if both are cleared, will it know to loose meta
     */
    public void testSetValueAndSetHidden() {
        seedSimpleSettingGroup();
        assertFalse(m_apple.isHidden());
        
        Folder meta = new Folder();
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());

        Setting copy = meta.decorate(m_root);
        Setting m_appleCopy = copy.getSetting("fruit").getSetting("apple");
        m_appleCopy.setHidden(true);
        m_appleCopy.setValue("macintosh");
        assertTrue(m_appleCopy.isHidden());
        assertEquals("macintosh", m_appleCopy.getValue());
        
        assertNotNull(meta.get(m_apple.getPath()));
        assertEquals(1, meta.size());
        
        // if matches default, should clear value
        m_appleCopy.setValue(null);
        m_appleCopy.setHidden(false);
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());        
    }
    
    public void testGetSettingViaRelativePath() {
        seedSimpleSettingGroup();
        assertSame(m_root, m_fruit.getSetting(".."));        
        assertSame(m_root, m_apple.getSetting("../.."));        
    }
    
    public void testGetSettingViaAbsoluePath() {
        seedSimpleSettingGroup();
        assertSame(m_root, m_fruit.getSetting("/"));        
        assertSame(m_root, m_fruit.getSetting("//"));        
        assertSame(m_fruit, m_apple.getSetting("/fruit"));        
        assertSame(m_apple, m_root.getSetting("/fruit/apple"));        
    }
    
    public void testGetSettingAbsoluteAndRelativePath() {
        seedSimpleSettingGroup();
        assertSame(m_fruit, m_root.getSetting("/fruit/../fruit"));        
        assertSame(m_root, m_apple.getSetting("../../fruit//"));        
    }

    public void test100GroupsWith100Settings() {
        SettingGroup root = new SettingGroup();
        for (int i = 0; i < 100; i++) {
            SettingGroup model = (SettingGroup) root.addSetting(new SettingGroup(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                model.addSetting(new SettingImpl(String.valueOf(j)));                
            }
            assertEquals(100, model.getValues().size());
        }
        assertEquals(100, root.getValues().size());
    }
}
