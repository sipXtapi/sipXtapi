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
package org.sipfoundry.sipxconfig.phone.cisco;

import org.apache.commons.lang.enums.Enum;

/**
 * Static differences in cisco models
 */
public final class CiscoModel extends Enum {

    public static final CiscoModel MODEL_7960 = new CiscoModel("cisco7960", 6);

    public static final CiscoModel MODEL_7940 = new CiscoModel("cisco7940", 2);

    /** analog phone adapter */
    public static final CiscoModel MODEL_ATA18X = new CiscoModel("ciscoAta18x", 2, "ata",
            "0x301,0x400,0x200");

    /** only public to comply with checkstyle private/public order req. */
    public static final String CP79XX = "cp79xx";

    /** standard phone */
    public static final CiscoModel MODEL_7905 = new CiscoModel("cisco7905", 1, "ld",
            "0x501,0x400,0x200");

    /** standard phone with switch */
    public static final CiscoModel MODEL_7912 = new CiscoModel("cisco7912", 1, "gk",
            "0x601,0x400,0x200");

    private static final CiscoModel UNKNOWN = new CiscoModel("unknown", 0);

    private String m_modelId;

    private int m_maxLines;

    private String m_cfgPrefix;

    private String m_upgCode;

    /* IP Models */
    private CiscoModel(String modelId, int maxLines) {
        super(modelId);
        setModelId(modelId);
        setMaxLines(maxLines);
    }

    /** ATA Models */
    private CiscoModel(String modelId, int maxLines, String cfgPrefix, String upgCode) {
        super(modelId);
        setModelId(modelId);
        setMaxLines(maxLines);
        setCfgPrefix(cfgPrefix);
        setUpgCode(upgCode);
    }

    public String getCfgPrefix() {
        return m_cfgPrefix;
    }

    public void setCfgPrefix(String cfgPrefix) {
        m_cfgPrefix = cfgPrefix;
    }

    public String getUpgCode() {
        return m_upgCode;
    }

    public void setUpgCode(String upgCode) {
        m_upgCode = upgCode;
    }

    public static CiscoModel getModel(String modelId) {
        CiscoModel model = (CiscoModel) getEnumMap(CiscoModel.class).get(modelId);
        return model == null ? UNKNOWN : model;
    }

    public int getMaxLines() {
        return m_maxLines;
    }

    public void setMaxLines(int maxLines) {
        m_maxLines = maxLines;
    }

    public String getModelId() {
        return m_modelId;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }
}
