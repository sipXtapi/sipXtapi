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
package org.sipfoundry.sipxconfig.vendor;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.settings.SettingSet;
import org.sipfoundry.sipxconfig.settings.SettingsContext;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000
 * conference phone
 */
public class PolycomPhone implements Phone {
    
    /** basic model */
    public static final String MODEL_300 = "polycom300";

    /** standard model */
    public static final String MODEL_500 = "polycom500";

    /** deluxe model */
    public static final String MODEL_600 = "polycom600";

    /** conference phone */
    public static final String MODEL_3000 = "polycom3000";
    
    private String m_id;
    
    private Endpoint m_endpoint;
    
    private PhoneContext m_phoneContext;
    
    private SettingsContext m_settingsContext;
    
    /**
     * @return Returns the settingsContext.
     */
    public SettingsContext getSettingsContext() {
        return m_settingsContext;
    }
    
    /**
     * @param settingsContext The settingsContext to set.
     */
    public void setSettingsContext(SettingsContext settingsContext) {
        m_settingsContext = settingsContext;
    }
    
    /**
     * @return Returns the phoneContext.
     */
    public PhoneContext getPhoneContext() {
        return m_phoneContext;
    }
    
    /**
     * @param phoneContext The phoneContext to set.
     */
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
    
    public String getModelId() {
        return m_id;
    }
    
    public void setModelId(String id) {
        m_id = id;
    }

    public String generateProfile(int profileIndex) {
        return "Profile #" + profileIndex;
    }

    public int getProfileCount() {
        return 1;
    }
    
    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
    
    public SettingSet getDefaultSettings() {
        SettingSet root = new SettingSet("root");
        root.addSetting(new SettingSet("network"));
        
        return root;        
    }
}
