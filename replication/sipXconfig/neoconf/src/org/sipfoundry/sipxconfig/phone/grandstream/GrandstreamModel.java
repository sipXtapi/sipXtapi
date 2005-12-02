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
package org.sipfoundry.sipxconfig.phone.grandstream;

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in grandstream models
 */
public final class GrandstreamModel extends PhoneModel {

    public static final GrandstreamModel MODEL_BUDGETONE = new GrandstreamModel(
            "BudgeTone", "Grandstream BudgeTone");

    public static final GrandstreamModel MODEL_HANDYTONE = new GrandstreamModel(
            "HandyTone", "Grandstream HandyTone");

    private GrandstreamModel(String modelId, String label) {
        super(GrandstreamPhone.BEAN_ID, modelId, label);
    }
}
