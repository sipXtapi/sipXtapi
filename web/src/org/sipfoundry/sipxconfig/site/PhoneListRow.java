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

import org.sipfoundry.sipxconfig.phone.PhoneSummary;

/**
 * Represent 1 row in the list of phones UI pulling information from endpoint and phone objects
 */
public class PhoneListRow extends PhoneSummary {
    
    private static final long serialVersionUID = 8968983300824869449L;
    
    private boolean m_selected;
           
    public boolean getSelected() {
        return m_selected;
    }
    
    public void setSelected(boolean selected) {
        m_selected = selected;
    }
}
