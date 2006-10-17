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
package org.sipfoundry.sipxconfig.phone.clearone;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class ClearoneModel extends PhoneModel {
    public ClearoneModel() {
        super(ClearonePhone.BEAN_ID);
        setMaxLineCount(1);
    }

    public ClearoneModel(String modelId) {
        this();
        setModelId(modelId);
    }
}
