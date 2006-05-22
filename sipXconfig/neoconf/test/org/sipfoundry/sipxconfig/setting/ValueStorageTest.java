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

public class ValueStorageTest extends TestCase {
    
    public void testSettingValue() {
        ValueStorage vs = new ValueStorage();
        String svalue = "red";
        Setting setting = new SettingImpl("parrot");
        vs.setSettingValue(setting, new SettingValueImpl(svalue), null);
        assertEquals(svalue, vs.getSettingValue(setting).getValue());
    }
    
    public void testNullValue() {
        ValueStorage vs = new ValueStorage();
        Setting setting = new SettingImpl("parrot");
        vs.setSettingValue(setting, new SettingValueImpl(null), null);
        assertNull(vs.getSettingValue(setting).getValue());        
    }
}
