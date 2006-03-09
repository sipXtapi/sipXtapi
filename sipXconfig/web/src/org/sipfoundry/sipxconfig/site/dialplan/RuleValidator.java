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
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.BaseValidator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

public class RuleValidator extends BaseValidator {

    public void validate(IFormComponent field, ValidationMessages messages, Object object) throws ValidatorException {
        IDialingRule rule = (IDialingRule) object;
        if (!rule.isEnabled()) {
            // only validate enabled rules
            return;
        }
        if (!rule.isInternal() && rule.getGateways().size() <= 0) {
            // rule is invalid - external rules have to have gateways
            rule.setEnabled(false);
            throw new ValidatorException(getMessage(), ValidationConstraint.CONSISTENCY);
        }
    }
}
