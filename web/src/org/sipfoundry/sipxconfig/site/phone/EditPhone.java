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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends BasePage {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public void ok(IRequestCycle cycle) {
        PhoneDao dao = PhonePageUtils.getPhoneContext(cycle).getPhoneDao(); 
        dao.storeEndpoint(getPhone().getEndpoint());
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneDao dao = PhonePageUtils.getPhoneContext(cycle).getPhoneDao(); 
        dao.storeEndpoint(getPhone().getEndpoint());
    }
    
    public void cancel(IRequestCycle cycle) {
        setPhone(null);
        cycle.activate(ManagePhones.PAGE);
    }
}
