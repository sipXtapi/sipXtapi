/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags;

class LightweightDeviceType {
    private String m_manufacturer;
    private String m_model;

    public LightweightDeviceType ( String manufacturer, String model ) {
        this.m_manufacturer = manufacturer;
        this.m_model = model;
    }

    public String getManufacturer () {
        return this.m_manufacturer;
    }

    public String getModel () {
        return this.m_model;
    }
} // LightweightDeviceType


