/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.text.MessageFormat;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneLabel extends BaseComponent {
    @Parameter(required = true)
    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    @Parameter(defaultValue = "true")
    public abstract boolean isRenderSerialNumber();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
    }

    public IPage editPhone(IRequestCycle cycle, String phoneSerialNumber) {
        // TODO: it would be better if we were serching by phone ID and not by serial number
        EditPhone editPhonePage = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Integer phoneId = getPhoneContext().getPhoneIdBySerialNumber(phoneSerialNumber);
        if (phoneId == null) {
            recordError("message.noPhoneWithSerialNumber", phoneSerialNumber);
            return null;
        }

        editPhonePage.setPhoneId(phoneId);
        editPhonePage.setReturnPage(getPage());
        return editPhonePage;
    }

    private void recordError(String messageId, String param) {
        IValidationDelegate delegate = TapestryUtils.getValidator(getPage());
        MessageFormat format = new MessageFormat(getMessages().getMessage(messageId));
        String message = format.format(new Object[] {
            param
        });
        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }
}
