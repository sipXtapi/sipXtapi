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
import org.sipfoundry.sipxconfig.phone.User;

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
        User user = getEndpoint().getUser(); 
        return user != null; 
    }
    
    public String getAssignedUser() {
        return isAssigned() ? "{unassigned}" : getEndpoint().getUser().getDisplayId();
    }
    
    public String getExtension() {
        return isAssigned() ? " " : getEndpoint().getUser().getExtension();
    }
    
}
