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

package com.pingtel.pds.profilewriter;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public abstract class ProfileWriterCommand {
    // These are teh 3 flavors of commands we support
    public static final int CREATE_PROFILE = 0;
    public static final int DELETE_PROFILE = 1;
    public static final int INSTALL_SOFTWARE = 2;
    public static final int UPGRADE_SOFTWARE = 3;
    public static final int INSTALL_DEVICE_SOFTWARE = 4;

    private int m_profileType;
    private int m_operation;
    private String m_macAddress;
    private String m_deviceURL;
    private String m_manufacturer;
    private String m_model;
    private String m_leafFileName;

    public ProfileWriterCommand ( int operation,
                              int profileType,
                              String macAddress,
                              String deviceURL,
                              String manufacturer,
                              String model,
                              String leafFileName ) {
        m_operation = operation;
        m_profileType = profileType;
        m_macAddress = macAddress;
        m_deviceURL = deviceURL;
        m_manufacturer = manufacturer;
        m_model = model;
        m_leafFileName = leafFileName;
    }


    public int getOperation() {
        return m_operation;
    }

    public int getProfileType() {
        return m_profileType;
    }

    public String getModel() {
        return m_model;
    }

    public String getDeviceURL() {
        return m_deviceURL;
    }

    public String getMacAddress() {
        return m_macAddress;
    }

    public String getManufacturer() {
        return m_manufacturer;
    }

    public String getLeafFileName() {
        return m_leafFileName;
    }
}
