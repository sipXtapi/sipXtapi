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
package org.sipfoundry.sipxconfig.site.dialplan;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class RuleValidator {
    private String m_gatewaysRequiredMessage;

    void validate(IDialingRule rule, IValidationDelegate delegate, IFormComponent component) {
        if (!rule.isEnabled()) {
            // only validate enabled rules
            return;
        }
        if (!rule.isInternal() && rule.getGateways().size() <= 0) {
            // rule is invalid - external rules have to have gateways
            delegate.setFormComponent(component);
            delegate.record(m_gatewaysRequiredMessage, ValidationConstraint.CONSISTENCY);
            rule.setEnabled(false);
        }
    }

    public void setGatewaysRequiredMessage(String gatewaysRequiredMessage) {
        m_gatewaysRequiredMessage = gatewaysRequiredMessage;
    }
}
