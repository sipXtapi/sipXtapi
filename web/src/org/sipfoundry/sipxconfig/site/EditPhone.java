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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends AbstractPhonePage implements PageRenderListener {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);    

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
    }

    public void save(IRequestCycle cycle) {
        getPhoneContext().getPhoneDao().storeEndpoint(getPhone().getEndpoint());
        cycle.activate(ListPhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ListPhones.PAGE);
    }
}
