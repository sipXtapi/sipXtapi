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

import org.sipfoundry.sipxconfig.core.Endpoint;

/**
 * Editing Endpoints 
 */
public class EditEndpoint {
    
    /**
     * delegate business object
     */
    private Endpoint m_endpoint = new Endpoint();
       
    /**
     * Save endpoint into the database
     * 
     * @return
     */
    public String save() {
        return "success";
    }
    
    /**
     * TODO: Delegate to endpoint object, invesigate better way
     * 
     * @return
     */
    public String getSerialNumber() {
        return m_endpoint.getSerialNumber();
    }
    
    /**
     * TODO: Delegate to endpoint object, invesigate better way
     * 
     * @return
     */
    public void setSerialNumber(String serialNumber) {
        m_endpoint.setSerialNumber(serialNumber);
    }
    
    public String getPhoneId() {
        return m_endpoint.getPhoneId();
    }
    
    public void setPhoneId(String phoneId) {
        m_endpoint.setPhoneId(phoneId);
    }
       
}
