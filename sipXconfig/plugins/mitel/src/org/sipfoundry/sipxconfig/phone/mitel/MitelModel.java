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
package org.sipfoundry.sipxconfig.phone.mitel;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class MitelModel extends PhoneModel {
    public MitelModel() {
        super(MitelPhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public MitelModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
