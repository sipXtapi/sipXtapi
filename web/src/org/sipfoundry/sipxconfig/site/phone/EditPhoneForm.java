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
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Editing a phone is reused for creating new phones and editing existing ones
 */
public abstract class EditPhoneForm extends BaseComponent {

    /**
     * all the phoneModels available to the system ready for UI selection 
     */
    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);
     
    public abstract PhoneContext getPhoneContext();
    
    /**
     * called before page is drawn
     */
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        PhoneContext phoneContext = getPhoneContext();
        String[] phoneIds = (String[]) phoneContext.getPhoneFactoryIds().toArray(new String[0]);
        setPhoneSelectionModel(new StringPropertySelectionModel(phoneIds));
    }        
}
