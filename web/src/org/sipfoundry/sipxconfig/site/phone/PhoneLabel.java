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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
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
    
    public void editPhone(IRequestCycle cycle) {
        EditPhone editPhonePage = (EditPhone) cycle.getPage(EditPhone.PAGE);
        Object[] params = cycle.getServiceParameters();
        String phoneSerialNumber = (String) TapestryUtils.assertParameter(String.class, params, 0);
        Integer phoneId = getPhoneContext().getPhoneIdBySerialNumber(phoneSerialNumber);
        if (phoneId == null) {
            recordError("message.noPhoneWithSerialNumber", phoneSerialNumber);
        } else {
            editPhonePage.setPhoneId(phoneId);       
            
            // When we navigate to the EditPhone page, clicking OK or Cancel on that
            // page should send the user back here
            ICallback callback = new PageCallback(getPage());
            editPhonePage.setCallback(callback);
    
            cycle.activate(editPhonePage);
        }
    }
    
    private void recordError(String messageId, String param) {
        IValidationDelegate delegate = TapestryUtils.getValidator((AbstractComponent) getPage());
        MessageFormat format = new MessageFormat(getMessage(messageId));
        String message = format.format(new Object[] {param});
        delegate.record(message, ValidationConstraint.CONSISTENCY);
    }

}
