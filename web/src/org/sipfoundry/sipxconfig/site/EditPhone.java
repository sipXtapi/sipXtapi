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
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Tapestry Page support for editing and creating new phone endpoints
 */
public class EditPhone extends AbstractPhonePage {
    
    private static final String NEXT_PAGE = "ListPhones";

    /** TODO: read from db if edit mode */
    private Endpoint m_endpoint = new Endpoint();
            
    private IPropertySelectionModel m_phoneModels;
    
    public void setPhoneContext(PhoneContext phoneContext) {
        super.setPhoneContext(phoneContext);
        String[] phoneIds = (String[]) phoneContext.getPhoneIds().toArray(new String[0]);
        m_phoneModels = new StringPropertySelectionModel(phoneIds);        
    }

    public Endpoint getEndpoint() {
        return m_endpoint;
    }
    
    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
    
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
    public IPropertySelectionModel getPhoneSelectionModel() {
        return m_phoneModels;
    }
}
