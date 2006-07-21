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
    
    /** basic phone */
    public static final PolycomModel MODEL_300 = new PolycomModel("300",
            "Polycom SoundPoint IP 300/301", 2);

    /** basic and deluxe phone */
    public static final PolycomModel MODEL_430 = new PolycomModel("430",
            "Polycom SoundPoint IP 430", 2);

    /** standard phone */
    public static final PolycomModel MODEL_500 = new PolycomModel("500",
            "Polycom SoundPoint IP 500/501", 3);

    /** deluxe phone, 6 normally but 12 lines if expansion module installed */
    public static final PolycomModel MODEL_600 = new PolycomModel("600",
            "Polycom SoundPoint IP 600/601", 12);

    /** conference phone, verify num lines */
    public static final PolycomModel MODEL_4000 = new PolycomModel("4000",
            "Polycom SoundPoint IP 4000", 1);
    
    public static final DeviceVersion VER_1_6 = new DeviceVersion("polycom1.6");

    public static final DeviceVersion VER_2_0 = new DeviceVersion("polycom2.0");

    private PolycomModel(String modelId, String label, int maxLines) {
        super(PolycomPhone.BEAN_ID, modelId, label, maxLines);
        setVersions(new DeviceVersion[] { 
            VER_1_6, 
            VER_2_0
        });
    }
}
