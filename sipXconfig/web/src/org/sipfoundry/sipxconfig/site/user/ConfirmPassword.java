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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ConfirmPassword extends BaseComponent {

    public abstract String getPassword();
    public abstract void setPassword(String password);

    public abstract String getConfirmPassword();
    public abstract void setConfirmPassword(String confirmPassword);
    
    public abstract String getPasswordMismatchMessage();
    public abstract void setPasswordMismatchMessage(String passwordMismatchMessage);
    
    // At rewind time, after the user has filled in the form, do any validation that spans
    // multiple fields.  Such validation cannot simply use a validator bound to a single field.
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        
        if (cycle.isRewinding()) {
            // Make sure the password and confirm password are non-null before proceeding.
            // Don't report an error because this condition is checked elsewhere.
            if (getPassword() == null || getConfirmPassword() == null) {
                return; 
            }

            // The user typed in the password twice.  Make sure that it was the same both times.
            IValidationDelegate delegate =
                TapestryUtils.getValidator((AbstractComponent) cycle.getPage());
            if (!getPassword().equals(getConfirmPassword())) {
                delegate.record(getPasswordMismatchMessage(), ValidationConstraint.CONSISTENCY);
                return;
            }
        }
    }

}
