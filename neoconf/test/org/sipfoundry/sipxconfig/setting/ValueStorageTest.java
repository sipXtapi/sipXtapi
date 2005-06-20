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

    private SettingGroup m_root;
    
    private Setting m_apple;

    public void testPreExistingValues() {
        seedSimpleSettingGroup();

        ValueStorage settingValues = new ValueStorage();
        settingValues.put(m_apple.getPath(), "granny smith");
        
        SettingGroup copy = (SettingGroup) settingValues.decorate(m_root);
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");

        assertEquals("granny smith", appleCopy.getValue());
        assertFalse(m_apple == appleCopy);
        assertEquals("/fruit/apple", appleCopy.getPath());
    }
    
    public void testNullSettings() {
        seedSimpleSettingGroup();

        SettingGroup copy = (SettingGroup) new ValueStorage().decorate(m_root);        
        Setting appleCopy = copy.getSetting("fruit").getSetting("apple");
        
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
        SettingGroup copy = (SettingGroup) storage.decorate(m_root);
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
    
    /**
     * Test interaction between filter runner and decorated setting group. If
     * decorator does not override acceptVisitor correctly, collection may contain
     * wrong instances. 
     */
    public void testFilterRunner() {
        seedSimpleSettingGroup();
        m_apple.setValue("granny smith");

        ValueStorage storage = new ValueStorage();
        SettingGroup copy = (SettingGroup) storage.decorate(m_root);
        copy.getSetting("fruit").getSetting("apple").setValue("macintosh");
        
        
        Collection c = FilterRunner.filter(APPLE, copy);
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
        
        Tag f = new Tag();
        Setting tagDecorated = f.decorate(m_root);
        tagDecorated.getSetting("fruit/apple").setValue("tag default");
        
        ValueStorage vs = new ValueStorage();
        Setting vsDecorated = vs.decorate(tagDecorated);
        vsDecorated.getSetting("fruit/apple").setValue("actual setting value");
                      
        Setting settingValue = vsDecorated.getSetting("fruit/apple");
        assertEquals("actual setting value", settingValue.getValue());

        String tagValue = (String) f.get(m_apple.getPath());
        assertEquals("tag default", tagValue);
        assertEquals(settingValue.getDefaultValue(), tagValue);

        assertEquals("system default", m_apple.getValue());
    }

    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingGroup("vegatables"));
    }   
}
