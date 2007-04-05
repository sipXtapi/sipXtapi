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
 * Resolving a setting value.  
 */
public interface SettingValueHandler {
    
    /**
     * call value.setValue() if implementation has a value for given setting 
     */
    public SettingValue getSettingValue(Setting setting); 

}
