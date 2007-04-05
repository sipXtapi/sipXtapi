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

public class SettingValueImplTest extends TestCase {
    
    public void testNullEquals() {
        SettingValue v1 = new SettingValueImpl(null);
        SettingValue v2 = new SettingValueImpl(null);
        assertTrue(v1.equals(v2));
    }

}
