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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.List;

import org.apache.commons.lang.enum.Enum;

/**
 * ConfigFileType
 */
public final class ConfigFileType extends Enum {
    public static final ConfigFileType MAPPING_RULES = new ConfigFileType("mappingrules.xml"); 
    public static final ConfigFileType FALLBACK_RULES = new ConfigFileType("fallbackrules.xml"); 
    public static final ConfigFileType AUTH_RULES = new ConfigFileType("authrules.xml");
    
    private ConfigFileType(String name) {
        super(name);
    }
    
    public static List getEnumList() {
        return getEnumList(ConfigFileType.class);
    }
}
