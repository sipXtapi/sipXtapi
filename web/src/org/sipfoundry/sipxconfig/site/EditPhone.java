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
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends AbstractPhonePage implements PageRenderListener {
    
    private static final String NEXT_PAGE = "ListPhones";
            
    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
        String[] phoneIds = (String[]) getPhoneContext().getPhoneIds().toArray(new String[0]);
        setPhoneSelectionModel(new StringPropertySelectionModel(phoneIds));
        
        // editing endpoint or...
        if (getEndpoint() == null) {
            // ...new endpoint
            setEndpoint(new Endpoint());
        }
    }    

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public void save(IRequestCycle cycle) {
        getPhoneContext().getPhoneDao().storeEndpoint(getEndpoint());
        cycle.activate(NEXT_PAGE);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(NEXT_PAGE);
    }

    /**
     * @return Returns all the phoneModels available to the system ready for UI selection 
     */
    public abstract IPropertySelectionModel getPhoneSelectionModel();

    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);
}
