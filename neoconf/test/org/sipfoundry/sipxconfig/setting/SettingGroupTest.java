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

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

public class SettingGroupTest extends TestCase {

    public void testDeepClone() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        root.addSetting(new SettingGroup("vegatables"));
        Setting apple = fruit.addSetting(new Setting("apple"));
        
        Map values = new HashMap();
        values.put(apple.getPath(), new SettingValue(apple.getPath(), "granny smith"));        
        
        SettingGroup rootClone = (SettingGroup) root.deepClone();
        rootClone.setSettingValues(values);
        Setting appleClone = rootClone.getSetting("fruit").getSetting("apple");
        
        // should be a cloned copy, not same instance
        assertFalse(apple == appleClone);
        assertEquals("granny smith", appleClone.getValue());
        assertEquals("/fruit/apple", appleClone.getPath());
    }
    
    public void testSetSettingValues() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        Setting apple = fruit.addSetting(new Setting("apple"));

        root.setSettingValues(null);
        // always returns something
        assertNotNull(root.getSettingValues());
        
        HashMap settingValues = new HashMap();
        root.setSettingValues(settingValues);
        assertNotNull(root.getSettingValues());
        
        apple.setValue("granny smith");
        SettingValue valuePreClear = (SettingValue) settingValues.get(apple.getPath());
        assertNotNull(valuePreClear);
        assertEquals("granny smith", valuePreClear.getValue());
        
        // operations on map will affect setting group because it should always
        // be kept in sync
        settingValues.clear();
        SettingValue valuePostClear = (SettingValue) settingValues.get(apple.getPath());
        assertNull(valuePostClear);
    }
    
    public void testSettingValues() {
        SettingGroup root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)root.addSetting(new SettingGroup("fruit"));
        Setting apple = fruit.addSetting(new Setting("apple", "granny smith"));
        
        Map values = root.getSettingValues();
        
        assertEquals(1, values.size());
        SettingValue value = (SettingValue) values.get(apple.getPath());
        assertNotNull(value);
        assertEquals("granny smith", value.getValue());        
    }
    
    /**
     * performance test
     */
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
