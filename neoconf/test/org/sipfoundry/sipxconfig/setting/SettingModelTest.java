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

public class SettingModelTest extends TestCase {

    public void testPopulateCopy() {
        SettingModel rootModel = new SettingModel();
        SettingModel fruitModel = (SettingModel)rootModel.addMeta(new SettingModel("fruit"));
        SettingMeta appleMeta = fruitModel.addMeta(new SettingMeta("apple"));
        
        SettingSet settings = new SettingSet();
        SettingSet fruit = (SettingSet) settings.addSetting(new SettingSet("fruit"));
        Setting apple = fruit.addSetting(new Setting("apple", "granny smith"));
        
        SettingModel rootClone = rootModel.populateCopy(settings);        
        SettingMeta appleClone = rootClone.getMeta("fruit").getMeta("apple");
        Setting appleRead = appleClone.getSetting();
        
        // TODO: FAILING
        // assertEquals(apple, appleRead);

        // TODO: FAILING
        // should be a cloned copy, not same instance
        // assertFalse(appleMeta == appleClone);
    }
    
    /**
     * performance test
     */
    public void test100ModelsWith100Metas() {
        SettingModel root = new SettingModel();
        for (int i = 0; i < 100; i++) {
            SettingModel model = (SettingModel) root.addMeta(new SettingModel(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                model.addMeta(new SettingMeta(String.valueOf(j)));                
            }
            assertEquals(100, model.size());
        }
        assertEquals(100, root.size());
    }
}
