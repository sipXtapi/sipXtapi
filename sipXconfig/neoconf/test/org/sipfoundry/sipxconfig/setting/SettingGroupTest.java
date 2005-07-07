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

    public void testModel() {
        seedSimpleSettingGroup();

        assertEquals("/fruit/apple", m_apple.getPath());        
        m_apple.setValue("granny smith");
        assertEquals("granny smith", m_apple.getValue());
        assertEquals("granny smith", m_apple.getDefaultValue());
    }
    
    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingGroup("vegatables"));
    }

    public void test100ModelsWith100Metas() {
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
