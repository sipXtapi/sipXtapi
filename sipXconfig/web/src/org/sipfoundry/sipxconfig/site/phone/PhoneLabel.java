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
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public abstract class PhoneLabel extends BaseComponent {

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    public abstract PhoneContext getPhoneContext();

    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
    }

    public IPage editPhone(IRequestCycle cycle, String phoneSerialNumber) {
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
