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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhonePage extends BasePage {
    
    public EditPhonePage() {
        // TODO: Read existing one
        setEndpoint(new Endpoint());
    }
    
    public abstract PhoneContext getPhoneContext();

    public abstract void setPhoneContext(PhoneContext phoneContext);

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public void save(IRequestCycle cycleTemp) {
        getPhoneContext().getPhoneDao().storeEndpoint(getEndpoint());
        // no-op avoid eclipse warning
        cycleTemp.getClass();
    }

    public void cancel(IRequestCycle cycleTemp) {
        // no-op avoid eclipse warning
        cycleTemp.getClass();
    }
}
