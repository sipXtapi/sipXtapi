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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.PhoneContext;


/**
 * First page of wizard-like UI for creating a new phone
 */
public abstract class NewPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "NewPhone"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public abstract PhoneContext getPhoneContext();

    public void finish(IRequestCycle cycle) {
        getPhoneContext().storeEndpoint(getEndpoint());
        cycle.activate(ManagePhones.PAGE);
    }

    public void next(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        // always fresh object
        Endpoint endpoint = new Endpoint();
        endpoint.setFolder(getPhoneContext().loadRootEndpointFolder());
        setEndpoint(endpoint);
    }
}
