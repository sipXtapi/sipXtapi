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

import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    private MappingRules m_mapping;
    private AuthRules m_auth;
    private FallbackRules m_fallback;

    public ConfigGenerator() {
        m_mapping = new MappingRules();
        m_auth = new AuthRules();
        m_fallback = new FallbackRules();
    }

    void generate(FlexibleDialPlan plan) {
        List rules = plan.getRules();

        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            if (!rule.isEnabled()) {
                return;
            }
            m_mapping.generate(rule);
            m_auth.generate(rule);
            m_fallback.generate(rule);
        }
    }

}
