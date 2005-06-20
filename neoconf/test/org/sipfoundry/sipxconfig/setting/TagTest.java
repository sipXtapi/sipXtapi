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
import java.util.Iterator;

import junit.framework.TestCase;


public class TagTest extends TestCase {

    private SettingGroup m_root;
    
    private Setting m_apple;
          
    /**
     * Tests that filter returns decorated settings from tag, not 
     * from underlying setting model.
     */
    public void testDecorateWithFilterRunner() {
        seedSimpleSettingGroup();
        Tag f = new Tag();
        Setting rootDecorated = f.decorate(m_root);
        Collection settings = FilterRunner.filter(SettingFilter.ALL, rootDecorated);
        assertEquals(4, settings.size());
        Iterator i = settings.iterator();
        i.next(); // fruit
        ((Setting) i.next()).setValue("granny smith");
        i.next(); // vegetable
        ((Setting) i.next()).setValue(null);
        assertNull(m_apple.getValue());
        assertEquals(1, f.getValues().size());
    }

    private void seedSimpleSettingGroup() {
        m_root = new SettingGroup();
        SettingGroup fruit = (SettingGroup)m_root.addSetting(new SettingGroup("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingGroup("vegetables")).addSetting(new SettingImpl("pea"));
    }
}
