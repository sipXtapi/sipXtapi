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
package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;

import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Generic phone does not correlate to any particular phone. It represents any unsupported phones
 * that simply needs to be configured with basic network settings to the commservers so it can
 * make a calls. Phone is responsible configuring itself matching the configuration on the
 * commservers
 */
public class GenericPhone implements Phone {

    public static final String GENERIC_PHONE_ID = "generic";

    private String m_id = GENERIC_PHONE_ID;

    private Endpoint m_endpoint;
    
    private SettingGroup m_settingGroup = new SettingGroup();
    
    public String getModelId() {
        return m_id;
    }

    public String getDisplayLabel() {
        return "Generic SIP Device";
    }

    public void setModelId(String id) {
        m_id = id;
    }

    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
    
    public Endpoint getEndpoint() {
        return m_endpoint;
    }

    /**
     * Return empty model, no custom settings
     */
    public SettingGroup getSettingGroup() {
        return m_settingGroup;
    }

    /**
     * Not applicable
     */
    public void generateProfiles(PhoneContext phoneContextTemp) throws IOException {        
    }
    
    /**
     * Not applicable
     */
    public void restart(PhoneContext phoneContextTemp) throws IOException {
    }
}