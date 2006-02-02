/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import org.easymock.MockControl;

public class SettingValueTest extends TestCase {
    private SettingImpl m_setting;

    protected void setUp() throws Exception {
        m_setting = new SettingImpl();
        m_setting.setName("s1");
        m_setting.setValue("v1");
    }

    public void testGetValue() {
        MockControl storageCtrl = MockControl.createControl(Storage.class);
        Storage storage = (Storage) storageCtrl.getMock();
        storage.getValue(m_setting);
        storageCtrl.setReturnValue("new_v1");
        storageCtrl.replay();

        SettingValue value = new SettingValue(storage, m_setting);
        assertEquals("v1", value.getDefaultValue());
        assertEquals("new_v1", value.getValue());
        storageCtrl.verify();
    }

    public void testSetValue() {
        MockControl storageCtrl = MockControl.createControl(Storage.class);
        Storage storage = (Storage) storageCtrl.getMock();
        storage.setValue(m_setting, "new_v1");
        storageCtrl.setReturnValue(null);
        storageCtrl.replay();

        SettingValue value = new SettingValue(storage, m_setting);
        value.setValue("new_v1");
        assertEquals("v1", m_setting.getValue());
        assertEquals("v1", value.getDefaultValue());
        storageCtrl.verify();
    }
}
