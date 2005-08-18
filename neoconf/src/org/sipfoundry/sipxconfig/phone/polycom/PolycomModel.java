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

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in polycom phone models 
 */
public final class PolycomModel extends PhoneModel {    

    /** basic phone */
    public static final PolycomModel MODEL_300 = new PolycomModel("300", "Polycom SoundPoint IP 300", 2);

    /** standard phone */
    public static final PolycomModel MODEL_500 = new PolycomModel("500", "Polycom SoundPoint IP 500", 4);

    /** deluxe phone */
    public static final PolycomModel MODEL_600 = new PolycomModel("600", "Polycom SoundPoint IP 600", 6);

    /** conference phone, verify num lines */
    public static final PolycomModel MODEL_3000 = new PolycomModel("3000", "Polycom SoundPoint IP 3000",  6);
    
    private PolycomModel(String modelId, String label, int maxLines) {
        super(PolycomPhone.BEAN_ID, modelId, label, maxLines);
    }
}
