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

import org.sipfoundry.sipxconfig.setting.SettingModel;



/**
 * Implement this to add support for new devices to the system
 */
public interface Phone {
    
    /**
     * every settings model for phones should have a lines settings model in the root model with
     * this name
     */
    public static final String LINE_SETTINGS = "line";

    public String getModelId();
    
    public void setModelId(String id);
    
    public String getDisplayLabel();

    public void setEndpoint(Endpoint endpoint);
    
    public Endpoint getEndpoint();
    
    public SettingModel getSettingModel();

    public void generateProfiles(PhoneContext phoneContext) throws IOException;
    
    public void restart(PhoneContext phoneContext) throws IOException;
}
