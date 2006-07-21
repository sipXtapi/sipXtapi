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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "NewPhone";

    public abstract Phone getPhone();

    public abstract PhoneModel getPhoneModel();

    public abstract void setPhoneModel(PhoneModel model);

    public abstract void setPhone(Phone phone);

    public abstract PhoneContext getPhoneContext();
    
    public abstract SettingDao getSettingDao();

    public abstract boolean isStay();
    
    public String finish() {
        IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
        if (delegate.getHasErrors()) {
            return null;
        }
        
        Phone phone = getPhone();        
        EditGroup.saveGroups(getSettingDao(), phone.getGroups());        
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
    
    public void pageBeginRender(PageEvent event) {
        Phone phone = getPhone();
        if (phone == null) {
            setPhone(getPhoneContext().newPhone(getPhoneModel()));            
        }
    }
}
