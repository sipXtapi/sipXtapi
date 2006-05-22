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

    public static final int LINECFG_PHONE = 1;

    public static final int LINECFG_HT = 2;

    public static final GrandstreamModel MODEL_PHONEBT = new GrandstreamModel(
            "PhoneBt", "Grandstream BudgeTone", 1, LINECFG_PHONE);

    public static final GrandstreamModel MODEL_PHONEGXP = new GrandstreamModel(
            "PhoneGxp", "Grandstream GXP2000", 4, LINECFG_PHONE);

    public static final GrandstreamModel MODEL_HT286 = new GrandstreamModel(
            "Ht286", "Grandstream HandyTone 286", 1, LINECFG_HT);

    public static final GrandstreamModel MODEL_HT386 = new GrandstreamModel(
            "Ht386", "Grandstream HandyTone 386", 2, LINECFG_HT);

    public static final GrandstreamModel MODEL_HT486 = new GrandstreamModel(
            "Ht486", "Grandstream HandyTone 486", 1, LINECFG_HT);

    public static final GrandstreamModel MODEL_HT488 = new GrandstreamModel(
            "Ht488", "Grandstream HandyTone 488", 2, LINECFG_HT);

    public static final GrandstreamModel MODEL_HT496 = new GrandstreamModel(
            "Ht496", "Grandstream HandyTone 496", 2, LINECFG_HT);

    private int m_lineCfgType;

    private GrandstreamModel(String modelId, String label, int maxLines, int lineCfgType) {
        super(GrandstreamPhone.BEAN_ID, modelId, label, maxLines);
        m_lineCfgType = lineCfgType;
    }

    public int getLineCfgType() {
        return m_lineCfgType;
    }    
    
    public boolean isHandyTone() {
        return getLineCfgType() == LINECFG_HT;
    }
}
