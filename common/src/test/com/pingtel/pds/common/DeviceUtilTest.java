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
package com.pingtel.pds.common;

import junit.framework.TestCase;

/**
 * Comments
 */
public class DeviceUtilTest extends TestCase {
    
    public void testDeviceProfileName() {        
        String pingerConfig = DeviceNameUtil.getInstance().getDeviceProfileName(
                PDSDefinitions.PROF_TYPE_PHONE, 
                PDSDefinitions.MANU_PINGTEL,
                PDSDefinitions.MODEL_SOFTPHONE_WIN, 
                "000000000000");
        assertEquals(pingerConfig, "pingtel/ixpressa_x86_win32/000000000000/pinger-config");
    }
    
    public void testDeviceProfileToken() {
        String pingerConfig = DeviceNameUtil.getInstance().getDeviceProfileToken(
                PDSDefinitions.PROF_TYPE_PHONE, 
                PDSDefinitions.MANU_PINGTEL);
        assertEquals(pingerConfig, "x-xpressa-device");
    }
}
