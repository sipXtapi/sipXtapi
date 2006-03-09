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

import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;

/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends BasePage {

    public static final String PAGE = "NewPhone";

    public abstract Phone getPhone();

    public abstract PhoneModel getPhoneModel();

    public abstract void setPhoneModel(PhoneModel model);

    public abstract void setPhone(Phone phone);

    public abstract PhoneContext getPhoneContext();

    public abstract boolean isStay();

    public String finish() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        if (delegate.getHasErrors()) {
            return null;
        }
        Phone prototype = getPhone();
        Phone phone = getPhoneContext().newPhone(getPhoneModel());

        // copy over phone settings into subclass's settings.
        // better way aluding me because we cannot create a concrete
        // phone object until a phone type is selected.
        phone.setSerialNumber(prototype.getSerialNumber());
        phone.setGroups(prototype.getGroups());
        phone.setDescription(prototype.getDescription());

        getPhoneContext().storePhone(phone);

        if (isStay()) {
            // triggers form to clear
            Phone nextPhone = getPhoneContext().newPhone(getPhoneModel());
            setPhone(nextPhone);
            return PAGE;
        }
        return ManagePhones.PAGE;
    }

    public String next() {
        return ManagePhones.PAGE;
    }

    public String cancel() {
        return ManagePhones.PAGE;
    }
}
