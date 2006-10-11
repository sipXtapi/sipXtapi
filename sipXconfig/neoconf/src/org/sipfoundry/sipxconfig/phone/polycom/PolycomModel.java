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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in polycom phone models
 */
public final class PolycomModel extends PhoneModel {
    
    /** Firmware 1.6 */
    public static final DeviceVersion VER_1_6 = new DeviceVersion(PolycomPhone.BEAN_ID, "1.6");

    /** Firmware 2.0 */
    public static final DeviceVersion VER_2_0 = new DeviceVersion(PolycomPhone.BEAN_ID, "2.0");

    
    public PolycomModel() {
        setBeanId(PolycomPhone.BEAN_ID);
        setVersions(new DeviceVersion[] { 
            VER_1_6, 
            VER_2_0
        });
    }
    
    public PolycomModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
