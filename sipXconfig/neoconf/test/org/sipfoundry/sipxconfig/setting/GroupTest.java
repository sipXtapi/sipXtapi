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
    
    private SettingImpl m_pea;
    
    protected void setUp() {
        m_root = new SettingSet();
        SettingSet fruit = (SettingSet)m_root.addSetting(new SettingSet("fruit"));
        m_apple = fruit.addSetting(new SettingImpl("apple"));
        SettingSet vegetable = (SettingSet)m_root.addSetting(new SettingSet("vegetable"));
        m_pea = (SettingImpl) vegetable.addSetting(new SettingImpl("pea"));
        m_pea.setValue("snowpea");
    }
          
    /**
     * Tests that filter returns decorated settings from tag, not 
     * from underlying setting model.
     */
    public void testDecorateWithFilterRunner() {
        Group f = new Group();
        f.decorate(m_root);
        Collection settings = SettingUtil.filter(SettingFilter.ALL, m_root);
        assertEquals(4, settings.size());
        Iterator i = settings.iterator();
        i.next(); // fruit
        ((Setting) i.next()).setValue("granny smith");
        i.next(); // vegetable
        ((Setting) i.next()).setValue("snowpea");
        assertNull(m_apple.getValue());
        assertEquals(1, f.getValues().size());
    }
    
    public void testDecorate() {
        Group f = new Group();
        f.getValues().put("/fruit/apple", "granny smith");
        f.decorate(m_root);
        assertEquals("granny smith", m_root.getSetting("fruit/apple").getValue());
        assertEquals("snowpea", m_root.getSetting("vegetable/pea").getValue());
    }
    
    public void testCompare() {
        assertEquals(0, new Group().compareTo(new Group()));
        
        Group g1 = new Group();
        g1.setName("a");
        Group g2 = new Group();
        g2.setName("b");
        assertTrue(g1.compareTo(g2) < 0);
        
        g1.setWeight(new Integer(5));
        g2.setWeight(new Integer(3));
        assertTrue(g1.compareTo(g2) > 0);
    }
}
