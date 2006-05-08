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

public class BeanWithSettingTest extends TestCase {
    
    public void testGetSettingValue() {
        BeanWithSettings bean = new BeanWithSettings();
        ValueStorage vs = new ValueStorage();
        vs.put("/bird/towhee", "rufous-sided");
        bean.setValueStorage(vs);
        assertEquals("rufous-sided", bean.getSettingModel2().getSettingValue("/bird/towhee"));
    }

}
