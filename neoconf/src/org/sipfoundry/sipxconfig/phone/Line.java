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

import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Line on a phone
 */
public interface Line {
    
    public Phone getPhone();
    
    public void setPhone(Phone phone);
    
    public void setLineMetaData(LineMetaData meta);

    public LineMetaData getLineMetaData();

    public Setting getSettingModel();

    public Setting getSettings();
}
