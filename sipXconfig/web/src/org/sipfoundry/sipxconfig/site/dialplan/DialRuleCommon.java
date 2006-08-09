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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * EditDialRule
 */
public abstract class DialRuleCommon extends BaseComponent {
    public abstract DialPlanContext getDialPlanContext();

    public abstract void setRuleId(Integer ruleId);

    public abstract DialingRule getRule();

    public abstract void setRule(DialingRule rule);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public abstract RuleValidator getValidRule();

    public abstract boolean isRenderGateways();

    public abstract boolean isRuleChanged();

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        try {
            IComponent component = getComponent("common");
            RuleValidator ruleValidator = getValidRule();
            IFormComponent enabled = (IFormComponent) component.getComponent("enabled");
            ruleValidator.validate(enabled, null, getRule());
        } catch (ValidatorException e) {
            delegate.record(e);
        }
        return !delegate.getHasErrors();
    }

    public void commit() {
        if (isValid()) {
            saveValid();
        }
    }

    private void saveValid() {
        DialingRule rule = getRule();
        getDialPlanContext().storeRule(rule);
        Integer id = getRule().getId();
        setRuleId(id);
    }

    public void formSubmit() {
        boolean renderGateways = isRenderGateways();
        boolean ruleChanged = isRuleChanged();
        boolean valid = isValid();
        if (valid && renderGateways && ruleChanged) {
            saveValid();
        }
    }
}
