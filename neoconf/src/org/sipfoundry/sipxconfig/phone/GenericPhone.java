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

import org.sipfoundry.sipxconfig.settings.PhoneSettings;

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

    private PhoneContext m_phoneContext;

    public String getModelId() {
        return m_id;
    }

    public void setModelId(String id) {
        m_id = id;
    }

    /**
     * No profiles
     */
    public String generateProfile(int profileIndexTemp) {
        return "";
    }

    /**
     * No profiles because there is never any output
     */
    public int getProfileCount() {
        return 0;
    }

    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }

    public AbstractSettings getSettings(SettingSet settings) {
        return new PhoneSettings(settings);
    }
}