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

public class SettingGroupTest extends TestCase {
    
    private SettingGroup m_root;
    
    private Setting m_apple;

    public void testImmutableModel() {
        seedSimpleSettingGroup();

        assertEquals("/fruit/apple", m_apple.getPath());
        assertNull(m_root.getValueStorage());
        
        try {
            m_apple.setValue("granny smith");
            fail();
        }
        catch (UnsupportedOperationException e) {
            assertTrue(true);
        }
    }
    
    public void testValueStorage() {
        seedSimpleSettingGroup();

        ValueStorage settingValues = new ValueStorage();
        settingValues.put(m_apple.getPath(), "granny smith");
        
        SettingGroup copy = (SettingGroup) m_root.getCopy(settingValues);
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");

        assertEquals("granny smith", appleCopy.getValue());
        assertFalse(m_apple == appleCopy);
        assertEquals("/fruit/apple", appleCopy.getPath());
    }
    
    public void testNullSettings() {
        seedSimpleSettingGroup();

        SettingGroup copy = (SettingGroup) m_root.getCopy(new ValueStorage());
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");
        
        assertNull(appleCopy.getValue());
        
        appleCopy.setValue("granny smith");
        assertEquals("granny smith", appleCopy.getValue());

        appleCopy.setValue(null);
        assertNull(appleCopy.getValue());
    }
    
    public void testSetValue() {
        seedSimpleSettingGroup();
        m_apple.setDefaultValue("granny smith");
        
        ValueStorage storage = new ValueStorage();
        SettingGroup copy = (SettingGroup) m_root.getCopy(storage);
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");
        
        assertEquals("granny smith", appleCopy.getValue());        
        assertNull(storage.get(appleCopy.getPath())); // white box
        
        appleCopy.setValue("macintosh");
        assertEquals("macintosh", appleCopy.getValue());
        assertNotNull(storage.get(appleCopy.getPath())); // white box

        appleCopy.setValue(null);
        assertEquals(appleCopy.getValue(), Setting.NULL_VALUE);
        assertNotNull(storage.get(appleCopy.getPath())); // white box
    }
    
    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = fruit.addSetting(new Setting("apple"));
        m_root.addSetting(new SettingGroup("vegatables"));
    }

    public void test100ModelsWith100Metas() {
        SettingGroup root = new SettingGroup();
        for (int i = 0; i < 100; i++) {
            SettingGroup model = (SettingGroup) root.addSetting(new SettingGroup(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                model.addSetting(new Setting(String.valueOf(j)));                
            }
            assertEquals(100, model.size());
        }
        assertEquals(100, root.size());
    }
}
