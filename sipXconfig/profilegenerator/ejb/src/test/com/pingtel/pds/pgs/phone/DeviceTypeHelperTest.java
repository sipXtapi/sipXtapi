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
package com.pingtel.pds.pgs.phone;

import junit.framework.TestCase;


public class DeviceTypeHelperTest extends TestCase 
{

    public void testGetLabel() 
    {
        // admittingly this test is fragile, because any changes to local
        // strings will fail this, but this is simplest way to test all the
        // supporting classes work without using all the supported classes
        assertEquals("A label that is overridden", "Cisco - 7905,7912 and 7940", 
                DeviceTypeHelper.getLabel("7940", "Cisco"));

        assertEquals("A label that is not overridden", "Hocus - Pocus", 
                DeviceTypeHelper.getLabel("Pocus", "Hocus"));
    }
}