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

public interface SettingModel {
    public SettingValue getSettingValue(Setting setting);

    public SettingValue getDefaultSettingValue(Setting setting);

    public void setSettingValue(Setting setting, String value);

    public SettingValue getProfileName(Setting setting);
}
