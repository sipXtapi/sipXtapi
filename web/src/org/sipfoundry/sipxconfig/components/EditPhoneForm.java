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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Comments
 */
public abstract class EditPhoneForm extends BaseComponent implements PageRenderListener {
    
    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);    

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);    

    public abstract PhoneContext getPhoneContext();

    public abstract void setPhoneContext(PhoneContext phoneContext);

    /**
     * @return Returns all the phoneModels available to the system ready for UI selection 
     */
    public abstract IPropertySelectionModel getPhoneSelectionModel();

    public abstract void setPhoneSelectionModel(IPropertySelectionModel phoneModels);

    /**
     * called before page is drawn
     */
    public void pageBeginRender(PageEvent eventTemp) {
/*        
        String[] phoneIds = (String[]) getPhoneContext().getPhoneIds().toArray(new String[0]);
        setPhoneSelectionModel(new StringPropertySelectionModel(phoneIds));
        if (getEndpoint() == null && getPhone() != null) {
            setEndpoint(getPhone().getEndpoint());
        }
*/
        setEndpoint(new Endpoint());
        String[] temp = new String[] { 
            "apple", "banana" 
        };
        setPhoneSelectionModel(new StringPropertySelectionModel(temp));
    }
        
}
