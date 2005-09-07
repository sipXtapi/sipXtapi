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

import org.apache.tapestry.IRequestCycle;
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

    public void finish(IRequestCycle cycle) {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        boolean save = !delegate.getHasErrors();
        if (save) {
            Phone prototype = getPhone();
            Phone phone = getPhoneContext().newPhone(getPhoneModel());
            
            // copy over phone settings into subclass's settings.
            // better way aluding me because we cannot create a concrete
            // phone object until a phone type is selected.
            phone.setSerialNumber(prototype.getSerialNumber());
            phone.setName(prototype.getName());
            phone.setGroups(prototype.getGroups());
            
            getPhoneContext().storePhone(phone);        

            if (isStay()) {
                // triggers form to clear
                Phone nextPhone = getPhoneContext().newPhone(getPhoneModel());
                setPhone(nextPhone);
                cycle.activate(PAGE);                    
            } else {
                cycle.activate(ManagePhones.PAGE);
            }
        }
    }
    
    public void next(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
}
