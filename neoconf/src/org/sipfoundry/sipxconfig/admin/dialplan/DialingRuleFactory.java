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

/**
 * DialingRuleFactory
 */
public class DialingRuleFactory {
    private static final Map PROTOTYPES = new HashMap();

    static {
        PROTOTYPES.put(DialingRule.Type.CUSTOM, new CustomDialingRule());
        PROTOTYPES.put(DialingRule.Type.INTERNAL, new InternalRule());
    }

    /**
     * Constructs dialing rule by cloning prototypes.
     * 
     * Throws illegal argument exception if invalid or unregistered type is passed.
     * @param type dialing rule type
     * @return newly created object
     */
    public DialingRule create(DialingRule.Type type) {
        BeanWithId proto = (BeanWithId) PROTOTYPES.get(type);
        if (null == proto) {
            throw new IllegalArgumentException("Illegal Dialing rule type: " + type);
        }
        return (DialingRule) proto.duplicate();
    }
}
