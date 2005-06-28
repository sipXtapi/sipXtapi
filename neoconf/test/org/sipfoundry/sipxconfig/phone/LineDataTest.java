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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class LineDataTest extends TestCase {

    /**
     * This tests that line groups are ignored in favor of using the phone groups instead
     */
    public void testLineGroupsIgnored() {
        PhoneData phone = new PhoneData();
        Group phoneGroup = new Group();
        phoneGroup.put("/bird", "sparrow");
        phone.addGroup(phoneGroup);

        LineData line = new LineData();
        Group lineGroup = new Group();
        lineGroup.put("/bird", "duck");
        line.addGroup(lineGroup);
        line.setPhoneData(phone);
        
        SettingSet settings = new SettingSet();
        settings.addSetting(new SettingImpl("bird"));
        Setting decorated = line.decorate(new Group(), settings); 
        assertEquals("sparrow", decorated.getSetting("bird").getValue());        
    }
}
