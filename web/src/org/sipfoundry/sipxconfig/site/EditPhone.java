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
import org.sipfoundry.sipxconfig.phone.PhoneDao;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public abstract class EditPhone extends AbstractPhonePage implements PageRenderListener {
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public abstract boolean getEditMode();
    
    public abstract void setEditMode(boolean editMode);
    
    /**
     * @return Returns all the phoneModels available to the system ready for UI selection 
     */
    public abstract IPropertySelectionModel getPhoneSelectionModel();

    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
        String[] phoneIds = (String[]) getPhoneContext().getPhoneIds().toArray(new String[0]);
        setPhoneSelectionModel(new StringPropertySelectionModel(phoneIds));

        if (getEndpoint() == null) {
            throw new IllegalArgumentException("page error: endpoint object required");
        }
        setEditMode(getEndpoint().getId() != PhoneDao.UNSAVED_ID);
    }

    public void save(IRequestCycle cycle) {
        getPhoneContext().getPhoneDao().storeEndpoint(getEndpoint());
        cycle.activate(PAGE_LIST_PHONES);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(PAGE_LIST_PHONES);
    }

}
