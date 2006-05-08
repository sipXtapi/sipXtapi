/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class BeanWithGroupsTest extends TestCase {

    public void testGetSettingValueOneGroup() {
        BeanWithGroups bean = new BeanWithGroups();
        bean.setSettings(new SettingModel());
        
        Group g1 = new Group();
        g1.put("/bird/towhee", "rufous-sided");        
        bean.addGroup(g1);
        
        assertEquals("rufous-sided", bean.getSettingModel2().getSettingValue("/bird/towhee"));
    }

    public void testGetSettingValueMultipleGroup() {
        BeanWithGroups bean = new BeanWithGroups();        
        
        Group g1 = new Group();
        g1.put("/bird/towhee", "rufous-sided");        
        bean.addGroup(g1);
        
        Group g2 = new Group();
        g2.setWeight(new Integer(1));
        g2.put("/bird/towhee", "pacific");        
        bean.addGroup(g2);

        assertEquals("pacific", bean.getSettingModel2().getSettingValue("/bird/towhee"));
    }
}
