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
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;

/**
 * Aggregation of Phone and Endpoint data mostly to pull together for display to end user. All
 * data here comes from properties of other objects (i.e. you'd never want to save this to the
 * database).  It's abstract so MVC systems can populate data model with subclasses that have
 * additional properies related to viewing system. e.g, is this item selected from interface. 
 */
public class PhoneSummary implements Serializable {

    private static final long serialVersionUID = 1L;

    private Phone m_phone;

    private EndpointAssignment m_assignment;

    /**
     * @return Returns the assignment.
     */
    public EndpointAssignment getAssignment() {
        return m_assignment;
    }

    /**
     * @param assignment The assignment to set.
     */
    public void setAssignment(EndpointAssignment assignment) {
        m_assignment = assignment;
    }

    /**
     * @return Returns the phone.
     */
    public Phone getPhone() {
        return m_phone;
    }
    
    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public Endpoint getEndpoint() {
        return m_phone.getEndpoint();
    }

    public boolean isAssigned() {
        return m_assignment != null && m_assignment.getUser() != null;
    }

    public String getAssignedUser() {
        String label;
        if (isAssigned()) {
            StringBuffer sb = new StringBuffer();
            sb.append(m_assignment.getUser().getDisplayId());
            if (m_assignment.getLabel() != null) {
                sb.append(" (").append(m_assignment.getLabel()).append(')');
            }
            
            label = sb.toString();
            
        } else {
            label = "{unassigned}";
        }
        
        return label;
    }

    public String getExtension() {
        return isAssigned() ? m_assignment.getUser().getExtension() : "";
    }
}