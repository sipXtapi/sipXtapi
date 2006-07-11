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

import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * Static differences in cisco models
 */
public final class CiscoModel extends PhoneModel {
    /** only public to avoid checkstyle error */
    public static final String ATA_ID = "ata";

    public static final CiscoModel MODEL_7960 = new CiscoModel(CiscoIpPhone.BEAN_ID, "7960",
            "Cisco IP 7960", 6);

    public static final CiscoModel MODEL_7940 = new CiscoModel(CiscoIpPhone.BEAN_ID, "7940",
            "Cisco IP 7940", 2);

    /** analog phone adapter */
    public static final CiscoModel MODEL_ATA18X = new CiscoModel(CiscoAtaPhone.BEAN_ID, "18x",
            "Cisco ATA 186/188", 2, ATA_ID, "0x301,0x400,0x200");

    /** only public to comply with checkstyle private/public order req. */
    public static final String CP79XX = "cp79xx";

    /** standard phone */
    public static final CiscoModel MODEL_7905 = new CiscoModel(CiscoAtaPhone.BEAN_ID, "7905",
            "Cisco IP 7905", 1, "ld", "0x501,0x400,0x200");

    /** standard phone with switch */
    public static final CiscoModel MODEL_7912 = new CiscoModel(CiscoAtaPhone.BEAN_ID, "7912",
            "Cisco IP 7912", 1, "gk", "0x601,0x400,0x200");
   
    private String m_cfgPrefix;

    private String m_upgCode;

    /* IP Models */
    private CiscoModel(String beanId, String modelId, String label, int maxLines) {
        super(beanId, modelId, label, maxLines);
    }

    /** ATA Models */
    private CiscoModel(String beanId, String modelId, String label, int maxLines,
            String cfgPrefix, String upgCode) {
        this(beanId, modelId, label, maxLines);
        m_cfgPrefix = cfgPrefix;
        m_upgCode = upgCode;
    }

    public String getCfgPrefix() {
        return m_cfgPrefix;
    }

    public String getUpgCode() {
        return m_upgCode;
    }
    
    public boolean isAta() {
        return ATA_ID.equals(getCfgPrefix());
    }
}
