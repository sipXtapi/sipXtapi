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

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

/**
 * DialingRuleFactory TODO: we should be able to use spring for that...
 */
public class DialingRuleFactory implements BeanFactoryAware {
    private static final Map<DialingRuleType, String> PROTOTYPES = new HashMap<DialingRuleType, String>();
    private BeanFactory m_beanFactory;

    static {
        PROTOTYPES.put(DialingRuleType.CUSTOM, "defaultCustomRule");
        PROTOTYPES.put(DialingRuleType.INTERNAL, "defaultInternalRule");
        PROTOTYPES.put(DialingRuleType.LONG_DISTANCE, "defaultLongDistanceRule");
        PROTOTYPES.put(DialingRuleType.LOCAL, "defaultLocalRule");
        PROTOTYPES.put(DialingRuleType.EMERGENCY, "defaultEmergencyRule");
        PROTOTYPES.put(DialingRuleType.INTERNATIONAL, "defaultInternationalRule");
        PROTOTYPES.put(DialingRuleType.ATTENDANT, "defaultAttendantRule");
    }

    /**
     * Constructs dialing rule from prototypes defined in Spring configuration file.
     * 
     * Throws illegal argument exception if invalid or unregistered type is passed.
     * 
     * @param type dialing rule type
     * @return newly created object
     */
    public DialingRule create(DialingRuleType type) {
        String beanId = PROTOTYPES.get(type);
        if (null == beanId) {
            throw new IllegalArgumentException("Illegal Dialing rule type: " + type);
        }
        DialingRule rule = (DialingRule) m_beanFactory.getBean(beanId, DialingRule.class);
        // reset new rule - we do not want to suggest invalid values for name, description etc.
        rule.setEnabled(false);
        rule.setDescription(StringUtils.EMPTY);
        rule.setName(StringUtils.EMPTY);
        return rule;
    }

    public Collection<DialingRuleType> getTypes() {
        return PROTOTYPES.keySet();
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }
}
