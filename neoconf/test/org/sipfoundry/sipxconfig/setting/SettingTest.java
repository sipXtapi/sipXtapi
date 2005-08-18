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

    private SettingSet m_root;

    private SettingImpl m_apple;

    private SettingImpl m_fruit;

    private void seedSimpleSettingGroup() {
        m_root = new SettingSet();
        m_fruit = (SettingSet) m_root.addSetting(new SettingSet("fruit"));
        m_apple = (SettingImpl) m_fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingSet("vegatables"));
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

        Group meta = new Group();
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());

        meta.decorate(m_root);
        Setting m_appleCopy = m_root.getSetting("fruit").getSetting("apple");
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

    public void testSetAdvanced() {
        seedSimpleSettingGroup();
        assertFalse(m_apple.isAdvanced());

        Group tag = new Group();
        assertNull(tag.get(m_apple.getPath()));
        assertEquals(0, tag.size());

        tag.decorate(m_root);
        Setting appleCopy = m_root.getSetting("fruit").getSetting("apple");
        m_apple.setAdvanced(true);
        assertTrue(appleCopy.isAdvanced());

        // Tag's used to save isAdvanced, but not anymore so this
        // test is not that exciting, but still worthy of existance.
    }

    /**
     * If both are set, will it know to keep meta, if both are cleared, will it know to loose meta
     */
    public void testSetValueAndSetHidden() {
        seedSimpleSettingGroup();
        assertFalse(m_apple.isAdvanced());

        Group meta = new Group();
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());

        meta.decorate(m_root);
        Setting appleCopy = m_root.getSetting("fruit").getSetting("apple");
        m_apple.setAdvanced(true);
        appleCopy.setValue("macintosh");
        assertTrue(appleCopy.isAdvanced());
        assertEquals("macintosh", appleCopy.getValue());

        assertNotNull(meta.get(m_apple.getPath()));
        assertEquals(1, meta.size());

        // if matches default, should clear value
        appleCopy.setValue(null);
        m_apple.setAdvanced(false);
        assertNull(meta.get(m_apple.getPath()));
        assertEquals(0, meta.size());
    }

    public void test100GroupsWith100Settings() {
        SettingSet root = new SettingSet();
        for (int i = 0; i < 100; i++) {
            SettingSet model = (SettingSet) root.addSetting(new SettingSet(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                model.addSetting(new SettingImpl(String.valueOf(j)));
            }
            assertEquals(100, model.getValues().size());
        }
        assertEquals(100, root.getValues().size());
    }

    public void testMergeChildren() {
        seedSimpleSettingGroup();
        SettingSet anotherFruit = new SettingSet("fruit");
        Setting banana = new SettingImpl("banana");
        anotherFruit.addSetting(banana);
        m_root.addSetting(anotherFruit);
        assertSame(anotherFruit, m_root.getSetting("fruit"));
        assertSame(banana, anotherFruit.getSetting("banana"));
        assertSame(m_apple, anotherFruit.getSetting("apple"));
    }    
    
}
