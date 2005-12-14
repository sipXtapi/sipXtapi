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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * DialingRuleFactory
 * TODO: we should be able to use spring for that...
 */
public class DialingRuleFactory {
    private static final Map PROTOTYPES = new HashMap();

    static {
        PROTOTYPES.put(DialingRuleType.CUSTOM, new CustomDialingRule());
        PROTOTYPES.put(DialingRuleType.INTERNAL, new InternalRule());
        PROTOTYPES.put(DialingRuleType.LONG_DISTANCE, new LongDistanceRule());
        PROTOTYPES.put(DialingRuleType.LOCAL, new LocalRule());
        PROTOTYPES.put(DialingRuleType.EMERGENCY, new EmergencyRule());
        PROTOTYPES.put(DialingRuleType.INTERNATIONAL, new InternationalRule());
        PROTOTYPES.put(DialingRuleType.ATTENDANT, new AttendantDialingRule());
    }

    /**
     * Constructs dialing rule by cloning prototypes.
     * 
     * Throws illegal argument exception if invalid or unregistered type is passed.
     * @param type dialing rule type
     * @return newly created object
     */
    public DialingRule create(DialingRuleType type) {
        BeanWithId proto = (BeanWithId) PROTOTYPES.get(type);
        if (null == proto) {
            throw new IllegalArgumentException("Illegal Dialing rule type: " + type);
        }
        return (DialingRule) proto.duplicate();
    }
}
