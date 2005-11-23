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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class FirmwareTest extends TestCase {
    public void testGetSettingModel() {
        Firmware f = new Firmware();
        f.setModelFilesContext(TestHelper.getModelFilesContext());
        f.setManufacturer(FirmwareManufacturer.UNMANAGED);
        assertNotNull(f.getSettings());
    }
}
