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
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditPhone"; 

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    /** REQUIRED PROPERTY */
    public abstract int getEndpointId();
    
    public abstract void setEndpointId(int id);
    
    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle); 
        dao.storeEndpoint(getEndpoint());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        setEndpoint(context.loadEndpoint(getEndpointId()));
    }       
}
