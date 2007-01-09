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

import org.apache.commons.lang.enums.Enum;

/**
 * Names used by this enumeration have to corresponds to names used in resource.xml file in order
 * for replication to work properly.
 */
public final class ConfigFileType extends Enum {
    public static final ConfigFileType MAPPING_RULES = new ConfigFileType("mappingrules.xml");
    public static final ConfigFileType FORWARDING_RULES = new ConfigFileType("forwardingrules.xml");
    public static final ConfigFileType FALLBACK_RULES = new ConfigFileType("fallbackrules.xml");
    public static final ConfigFileType AUTH_RULES = new ConfigFileType("authrules.xml");
    public static final ConfigFileType ORBITS = new ConfigFileType("orbits.xml");
    public static final ConfigFileType E911_RULES = new ConfigFileType("e911rules.xml");
    public static final ConfigFileType CONFERENCES = new ConfigFileType("conferences.xml");
    public static final ConfigFileType ATTENDANT_SCHEDULE = new ConfigFileType("attendant_schedule.xml");
    public static final ConfigFileType ORGANIZATION_PREFS = new ConfigFileType("organizationprefs.xml");
    public static final ConfigFileType RESOURCE_LISTS = new ConfigFileType("resource-lists.xml");

    private ConfigFileType(String name) {
        super(name);
    }
}
