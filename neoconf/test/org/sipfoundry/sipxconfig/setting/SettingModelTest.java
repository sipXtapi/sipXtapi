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

    public void test100ModelsWith100Meta() {
        SettingModel root = new SettingModel();
        for (int i = 0; i < 100; i++) {
            SettingModel firstlevel = (SettingModel) root.addMeta(new SettingModel(String.valueOf(i)));
            for (int j = 0; j < 100; j++) {
                new SettingMeta(String.valueOf(i));                
            }
        }
    }
}
