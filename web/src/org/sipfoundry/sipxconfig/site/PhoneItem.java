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

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Represent 1 row in the list of phones UI pulling information from endpoint and phone objects
 */
public class PhoneItem  {
    
    private Phone m_phone;
    
    private boolean m_selected;
    
    public PhoneItem(Phone phone) {
        m_phone = phone;
    }

    /**
     * @return Returns the phone.
     */
    public Phone getPhone() {
        return m_phone;
    }
    
    public Endpoint getEndpoint() {
        return m_phone.getEndpoint();        
    }    
    
    public boolean getSelected() {
        return m_selected;
    }
    
    public void setSelected(boolean selected) {
        m_selected = selected;
    }
    
    public boolean isAssigned() {
        // todo
        return false;
    }
    
    public String getAssignedUser() {
        //todo
        return "{unassigned}";
    }
    
    public String getExtension() {
        // todo
        return " ";
    }
    
}
