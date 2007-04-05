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

/**
 * Allows you to dynamically come up w/profile anme
 */
public interface ProfileNameHandler {
    
    /**
     * @return null if you do not handle this setting and wish to use default
     */
    public SettingValue getProfileName(Setting setting);

}
