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
    
    private boolean m_applyPressed;
    
    public abstract ICallback getCallback();

    public abstract IActionListener getListener();

    public abstract String getSuccessMessage();

    public void onRefresh(IRequestCycle cycle_) {
        // do nothing, page should refresh when rendering
    }

    public void onOk(IRequestCycle cycle) {
        m_applyPressed = false; 
        apply(cycle);
        if (TapestryUtils.isValid((AbstractPage) getPage())) {
            getCallback().performCallback(cycle);
        }
    }

    public void onApply(IRequestCycle cycle) {
        m_applyPressed = true; 
        apply(cycle);
    }
    
    /**
     * Incase parent component needs to know if Apply was pressed instead of OK. Return is 
     * undetermined if called when any other button was pressed.
     * 
     * A more elegant, however slighlty more involved solution would add paramter
     * to FormActions, direction out, that called boolean setter on parent directory
     * similiar to button and tag releationship
     */
    public boolean wasApplyPressedInsteadOfOk() {
        return m_applyPressed;
    }
    
    private void apply(IRequestCycle cycle) {
        IValidationDelegate validator = TapestryUtils.getValidator((AbstractComponent) getPage());
        IActionListener listener = getListener();
        IActionListener adapter = new TapestryContext.UserExceptionAdapter(validator, listener);
        adapter.actionTriggered(this, cycle);
        if (validator instanceof SipxValidationDelegate) {
            SipxValidationDelegate sipxValidator = (SipxValidationDelegate) validator;
            String msg = StringUtils.defaultIfEmpty(getSuccessMessage(), getMessage("user.success"));
            sipxValidator.recordSuccess(msg);
        }        
    }

    public void onCancel(IRequestCycle cycle) {
        getCallback().performCallback(cycle);
    }
}
