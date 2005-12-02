/*
 *
 *
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 snom technology AG
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.snom;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

public final class SnomModel extends PhoneModel {

    public static final SnomModel MODEL_360 = new SnomModel("360", "Snom 360", 12);

    public static final SnomModel MODEL_320 = new SnomModel("320", "Snom 320", 12);

    private SnomModel(String modelId, String label, int maxLines) {
        super(SnomPhone.BEAN_ID, modelId, label, maxLines);
    }
}
