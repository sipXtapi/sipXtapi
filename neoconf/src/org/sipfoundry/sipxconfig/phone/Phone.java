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

import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Implement this to add support for new devices to the system
 */
public interface Phone {
    
    public String getModelId();
    
    public void setModelId(String id);
    
    public String getDisplayLabel();
    
    public Setting getSettingModel(Endpoint endpoint);

    public Setting getSettingModel(Line line);
    
    public void generateProfiles(Endpoint endpoint) throws IOException;
    
    public void restart(Endpoint endpoint) throws IOException;
}
