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
        BeanWithSettings bean = new BeanWithSettings();
        InputStream in = getClass().getResourceAsStream("birds.xml");
        bean.setSettingModel(TestHelper.loadSettings(in));
        String path = "towhee/rufous-sided";
        bean.setSettingValue(path, "4");
        assertEquals("4", bean.getSettingModel2().getSettingValue(path));
    }
}
