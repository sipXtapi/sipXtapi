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

import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Special type of falbackrules document with a single host match matching
 * standard SIPx hosts
 * 
 */
public class FallbackRules extends MappingRules {
    public FallbackRules() {
        // Initially identical to mapping rules
    }

    public void generate(IDialingRule rule) {
        if (!isInternal(rule)) {
            generateRule(rule);
        }
    }
}
