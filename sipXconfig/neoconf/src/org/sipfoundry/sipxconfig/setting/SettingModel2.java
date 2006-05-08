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

public interface SettingModel2  {
    /** temporary until settingmodel2 becomes only model */
    static interface Connector {
        public void connect(SettingModel2 model2);
    }
    public void addSettingValueHandler(SettingValueHandler handler);
    public String getSettingValue(String path);
}
