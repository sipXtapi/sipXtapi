/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class LgNortelModel extends PhoneModel {
    public LgNortelModel() {
        super(LgNortelPhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public LgNortelModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
