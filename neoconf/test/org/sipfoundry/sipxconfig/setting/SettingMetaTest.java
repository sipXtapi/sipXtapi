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


public class SettingMetaTest extends TestCase {

    private SettingGroup m_root;
    
    private Setting m_apple;

    public void testSetValue() {
        seedSimpleSettingGroup();
        assertNull(m_apple.getValue());
        
        MetaStorage meta = new MetaStorage();
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
        
        MetaStorage meta = new MetaStorage();
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
        
        MetaStorage meta = new MetaStorage();
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
    
    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingGroup("vegatables"));
    }
}
