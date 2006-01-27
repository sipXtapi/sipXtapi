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
package org.sipfoundry.sipxconfig.components;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.valid.IValidationDelegate;

public abstract class FormActions extends BaseComponent {

    public abstract ICallback getCallback();

    public abstract IActionListener getListener();

    public abstract String getSuccessMessage();

    public void onOk(IRequestCycle cycle) {
        apply(cycle);
        if (TapestryUtils.isValid((AbstractPage) getPage())) {
            getCallback().performCallback(cycle);
        }
    }

    public void onApply(IRequestCycle cycle) {
        apply(cycle);
    }

    private void apply(IRequestCycle cycle) {
        IValidationDelegate validator = TapestryUtils.getValidator((AbstractComponent) getPage());
        IActionListener listener = getListener();
        IActionListener adapter = new TapestryContext.UserExceptionAdapter(validator, listener);
        adapter.actionTriggered(this, cycle);
        if (validator instanceof SipxValidationDelegate) {
            SipxValidationDelegate sipxValidator = (SipxValidationDelegate) validator;
            String msg = StringUtils.defaultIfEmpty(getSuccessMessage(),
                    getMessage("user.success"));
            sipxValidator.recordSuccess(msg);
        }
    }

    public void onCancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
