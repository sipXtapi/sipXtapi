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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends AbstractPhonePage {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public void ok(IRequestCycle cycle) {
        getPhoneContext().getPhoneDao().storeEndpoint(getPhone().getEndpoint());
        cycle.activate(ListPhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        getPhoneContext().getPhoneDao().storeEndpoint(getPhone().getEndpoint());
    }
    
    public void cancel(IRequestCycle cycle) {
        setPhone(null);
        cycle.activate(ListPhones.PAGE);
    }
}
