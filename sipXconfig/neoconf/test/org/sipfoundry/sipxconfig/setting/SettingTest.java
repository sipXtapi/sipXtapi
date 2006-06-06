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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

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
        assertEquals("fruit/apple", m_apple.getPath());
        m_apple.setValue("granny smith");
        assertEquals("granny smith", m_apple.getValue());
        assertEquals("granny smith", m_apple.getDefaultValue());
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
    
    public void testGetProfileName() {
        SettingImpl s = new SettingImpl("bluejay");
        assertEquals("bluejay", s.getProfileName());        
        s.setProfileName("indigojay");
        assertEquals("indigojay", s.getProfileName());
    }    
    
    public void testGetProfileHandler() {
        SettingImpl s = new SettingImpl("bluejay");
        SettingValue2 originalValue = new SettingValueImpl("bluejay");
        SettingValue2 handlerValue = new SettingValueImpl("indigojay");
        
        IMocksControl modelCtrl = EasyMock.createStrictControl();
        SettingModel2 model = modelCtrl.createMock(SettingModel2.class);
        model.getProfileName(s, originalValue);
        modelCtrl.andReturn(handlerValue);
        modelCtrl.replay();

        s.setModel(model);
        assertSame("indigojay", s.getProfileName());

        modelCtrl.verify();
    }
}
