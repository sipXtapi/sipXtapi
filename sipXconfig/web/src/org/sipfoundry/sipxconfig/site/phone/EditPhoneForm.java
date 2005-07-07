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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.MapSelectionModel;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Editing a phone is reused for creating new phones and editing existing ones
 */
public abstract class EditPhoneForm extends BaseComponent implements PageRenderListener {

    /**
     * all the phoneModels available to the system ready for UI selection 
     */
    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);
     
    public abstract PhoneContext getPhoneContext();
        
    public void pageBeginRender(PageEvent event_) {
        PhoneContext phoneContext = getPhoneContext();
        setPhoneSelectionModel(new MapSelectionModel(phoneContext.getPhoneFactoryIds()));
    }    
}
