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
import org.sipfoundry.sipxconfig.phone.Endpoint;


/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends AbstractPhonePage {
    
    public static final String PAGE = "NewPhone"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);

    public void finish(IRequestCycle cycle) {
        getPhoneContext().getPhoneDao().storeEndpoint(getEndpoint());
        cycle.activate(ListPhones.PAGE);
    }

    public void next(IRequestCycle cycle) {
        cycle.activate(ListPhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListPhones.PAGE);
    }
}
