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
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.apache.commons.lang.enums.Enum;


public final class DialingRuleType extends Enum {
    public static final DialingRuleType INTERNATIONAL = new DialingRuleType("International");
    public static final DialingRuleType EMERGENCY = new DialingRuleType("Emergency");
    public static final DialingRuleType MAPPING_RULE = new DialingRuleType("Mapping Rule");
    public static final DialingRuleType CUSTOM = new DialingRuleType("Custom");
    public static final DialingRuleType LOCAL = new DialingRuleType("Local");
    public static final DialingRuleType INTERNAL = new DialingRuleType("Internal");
    public static final DialingRuleType LONG_DISTANCE = new DialingRuleType("Long Distance");
    public static final DialingRuleType RESTRICTED = new DialingRuleType("Restricted");
    public static final DialingRuleType TOLL_FREE = new DialingRuleType("Toll free");
    public static final DialingRuleType ATTENDANT = new DialingRuleType("Attendant");

    private DialingRuleType(String name) {
        super(name);
    }
}
