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

import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class BeanWithSettingTest extends TestCase {
    
    public void testGetSettingValue() throws Exception {
        BeanWithSettings bean = new BirdWithSettings();
        bean.setSettingValue("towhee/rufous-sided", "4");
        assertEquals("4", bean.getSettingValue("towhee/rufous-sided"));
    }

    static class BirdWithSettings extends BeanWithSettings {
        protected Setting loadSettings() {
            InputStream in = getClass().getResourceAsStream("birds.xml");            
            return TestHelper.loadSettings(in);
        }            

        @Override
        public void initialize() {            
        }
    }
}
