/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

public interface Storage extends SettingValueHandler {

    public void setSettingValue(Setting setting, SettingValue value, SettingValue defaultValue);

    public void revertSettingToDefault(Setting setting);
}
