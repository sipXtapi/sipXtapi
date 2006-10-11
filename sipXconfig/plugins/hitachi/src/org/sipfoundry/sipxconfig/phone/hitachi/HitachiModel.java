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
package org.sipfoundry.sipxconfig.phone.hitachi;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class HitachiModel extends PhoneModel {
    public HitachiModel() {
        super(HitachiPhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public HitachiModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
