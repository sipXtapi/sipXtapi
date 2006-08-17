/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public interface SettingModel2 {
    public SettingValue2 getSettingValue(Setting setting);

    public SettingValue2 getDefaultSettingValue(Setting setting);

    public void setSettingValue(Setting setting, String value);

    public SettingValue2 getProfileName(Setting setting);
}
