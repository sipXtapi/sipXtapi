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
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Editing a phone is reused for creating new phones and editing existing ones
 */
public abstract class EditPhoneForm extends BaseComponent {
    
    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);    

    /**
     * @return Returns all the phoneModels available to the system ready for UI selection 
     */
    public abstract IPropertySelectionModel getPhoneSelectionModel();

    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);
    
    /**
     * called before page is drawn
     */
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        PhoneContext phoneContext = PhonePageUtils.getPhoneContext(cycle);
        String[] phoneIds = (String[]) phoneContext.getPhoneIds().toArray(new String[0]);
        setPhoneSelectionModel(new StringPropertySelectionModel(phoneIds));
    }        
}
