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

    public void testImmutableModel() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        root.addSetting(new SettingGroup("vegatables"));
        Setting apple = fruit.addSetting(new Setting("apple"));

        assertEquals("/fruit/apple", apple.getPath());
        assertNull(root.getValueStorage());
        
        try {
            apple.setValue("granny smith");
            fail();
        }
        catch (UnsupportedOperationException e) {
            assertTrue(true);
        }
    }
    
    public void testValueStorage() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        Setting apple = fruit.addSetting(new Setting("apple"));

        ValueStorage settingValues = new ValueStorage();
        settingValues.put(apple.getPath(), new SettingValue(apple.getPath(), "granny smith"));
        
        SettingGroup copy = (SettingGroup) root.getCopy(settingValues);
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");

        assertEquals("granny smith", appleCopy.getValue());
        assertFalse(apple == appleCopy);
        assertEquals("/fruit/apple", appleCopy.getPath());
    }
    
    public void testNullSettings() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        Setting apple = fruit.addSetting(new Setting("apple"));

        SettingGroup copy = (SettingGroup) root.getCopy(new ValueStorage());
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");
        
        assertNull(appleCopy.getValue());
        
        appleCopy.setValue("granny smith");
        assertEquals("granny smith", appleCopy.getValue());

        appleCopy.setValue(null);
        assertNull(appleCopy.getValue());
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
