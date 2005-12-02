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


public class ValueStorageTest extends TestCase {

    private static SettingFilter APPLE = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {                
            return setting.getName().equals("apple");
        }
    };

    private SettingSet m_root;
    
    private Setting m_apple;

    public void testPreExistingValues() {
        seedSimpleSettingGroup();

        ValueStorage settingValues = new ValueStorage();
        settingValues.put(m_apple.getPath(), "granny smith");
        
        settingValues.decorate(m_root);
        Setting appleCopy = m_root.getSetting("fruit").getSetting("apple");

        assertEquals("granny smith", appleCopy.getValue());
        assertFalse(m_apple == appleCopy);
        assertEquals("/fruit/apple", appleCopy.getPath());
    }
    
    public void testNullSettings() {
        seedSimpleSettingGroup();

        new ValueStorage().decorate(m_root);        
        Setting appleCopy = m_root.getSetting("fruit").getSetting("apple");
        
        assertNull(appleCopy.getValue());
        
        appleCopy.setValue("granny smith");
        assertEquals("granny smith", appleCopy.getValue());

        appleCopy.setValue(null);
        assertNull(appleCopy.getValue());
    }
    
    public void testSetValue() {
        seedSimpleSettingGroup();
        m_apple.setValue("granny smith");
        
        ValueStorage storage = new ValueStorage();
        storage.decorate(m_root);
        Setting appleCopy = m_root.getSetting("fruit").getSetting("apple");
        
        assertEquals("granny smith", appleCopy.getValue());        
        assertNull(storage.get(appleCopy.getPath())); // white box
        
        appleCopy.setValue("macintosh");
        assertEquals("macintosh", appleCopy.getValue());
        assertNotNull(storage.get(appleCopy.getPath())); // white box

        appleCopy.setValue(null);
        assertEquals(appleCopy.getValue(), Setting.NULL_VALUE);
        assertNotNull(storage.get(appleCopy.getPath())); // white box
    }
    
    /**
     * Test interaction between filter runner and decorated setting group. If
     * decorator does not override acceptVisitor correctly, collection may contain
     * wrong instances. 
     */
    public void testFilterRunner() {
        seedSimpleSettingGroup();
        m_apple.setValue("granny smith");

        ValueStorage storage = new ValueStorage();
        storage.decorate(m_root);
        m_root.getSetting("fruit").getSetting("apple").setValue("macintosh");
        
        
        Collection c = SettingUtil.filter(APPLE, m_root);
        assertEquals(1, c.size());
        Setting actualApple = (Setting) c.iterator().next(); 
        assertEquals(actualApple.getValue(), "macintosh");        
    }
    
   /**
    *  ValueStorage that's decorating a Tag, that inturn is decorating a Model  
    */
    public void testTagAndValueStorageDecorated() {
        seedSimpleSettingGroup();
        m_apple.setValue("system default");
        
        Group f = new Group();
        f.decorate(m_root);
        m_root.getSetting("fruit/apple").setValue("tag default");
        
        ValueStorage vs = new ValueStorage();
        vs.decorate(m_root);
        m_root.getSetting("fruit/apple").setValue("actual setting value");
                      
        Setting settingValue = m_root.getSetting("fruit/apple");
        assertEquals("actual setting value", settingValue.getValue());

        String tagValue = (String) f.get(m_apple.getPath());
        assertEquals("tag default", tagValue);
        assertEquals(settingValue.getDefaultValue(), tagValue);

        assertEquals("system default", m_apple.getValue());
    }

    private void seedSimpleSettingGroup() {
        m_root = new SettingSet();
        SettingSet fruit = (SettingSet)m_root.addSetting(new SettingSet("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingSet("vegatables"));
    }   
}
