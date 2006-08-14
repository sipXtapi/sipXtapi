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

import org.sipfoundry.sipxconfig.setting.BeanWithGroupsTest.BirdWithGroups;

public class GroupTest extends TestCase {

    private SettingSet m_root;

    private SettingImpl m_pea;

    protected void setUp() {
        m_root = new SettingSet();
        SettingSet fruit = (SettingSet) m_root.addSetting(new SettingSet("fruit"));
        fruit.addSetting(new SettingImpl("apple"));
        SettingSet vegetable = (SettingSet) m_root.addSetting(new SettingSet("vegetable"));
        m_pea = (SettingImpl) vegetable.addSetting(new SettingImpl("pea"));
        m_pea.setValue("snowpea");
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

    public void testInherhitSettingsForEditing() {
        BirdWithGroups bird = new BirdWithGroups();
        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        settings.getSetting("towhee/canyon").setValue("12");

        // bird bean should remain untouched
        assertNull(bird.getSettingValue("towhee/canyon"));

        assertEquals("12", settings.getSetting("towhee/canyon").getValue());
        assertEquals(1, g.size());
    }

    public void testInherhitSettingsForEditingIgnoreMatchingDefaults() {
        BirdWithGroups bird = new BirdWithGroups();
        assertEquals("0", bird.getSettingValue("pigeon/passenger"));

        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        settings.getSetting("pigeon/passenger").setValue("0");

        assertEquals(0, g.size());
    }

    public void testInherhitSettingsForEditingGetDefaultValue() {
        BirdWithGroups bird = new BirdWithGroups();
        assertEquals("0", bird.getSettingValue("woodpecker/ivory-billed"));

        Group g = new Group();
        Setting settings = g.inherhitSettingsForEditing(bird);
        Setting ivoryBilled = settings.getSetting("woodpecker/ivory-billed");

        ivoryBilled.setValue("2");

        assertEquals("0", ivoryBilled.getDefaultValue());
        assertEquals("2", ivoryBilled.getValue());
    }
}
