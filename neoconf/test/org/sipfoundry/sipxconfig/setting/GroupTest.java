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


public class GroupTest extends TestCase {

    private SettingSet m_root;
    
    private Setting m_apple;
          
    /**
     * Tests that filter returns decorated settings from tag, not 
     * from underlying setting model.
     */
    public void testDecorateWithFilterRunner() {
        seedSimpleSettingGroup();
        Group f = new Group();
        f.decorate(m_root);
        Collection settings = SettingUtil.filter(SettingFilter.ALL, m_root);
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
        m_root = new SettingSet();
        SettingSet fruit = (SettingSet)m_root.addSetting(new SettingSet("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        m_root.addSetting(new SettingSet("vegetables")).addSetting(new SettingImpl("pea"));
    }
}
