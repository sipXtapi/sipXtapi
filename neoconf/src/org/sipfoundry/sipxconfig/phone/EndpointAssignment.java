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
 * Association between Users and their assigned phones.
 */
public class EndpointAssignment implements Serializable {

    private static final long serialVersionUID = 1L;
    
    private String m_label;
    
    private int m_id = -1;
    
    private User m_user;
    
    private Endpoint m_endpoint;


    /**
     * @return Returns the endpoint.
     */
    public Endpoint getEndpoint() {
        return m_endpoint;
    }
    /**
     * @param endpoint The endpoint to set.
     */
    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
    /**
     * @return Returns the id.
     */
    public int getId() {
        return m_id;
    }
    /**
     * @param id The id to set.
     */
    public void setId(int id) {
        m_id = id;
    }
    /**
     * @return Returns the label.
     */
    public String getLabel() {
        return m_label;
    }
    /**
     * @param label The label to set.
     */
    public void setLabel(String label) {
        m_label = label;
    }
    /**
     * @return Returns the user.
     */
    public User getUser() {
        return m_user;
    }
    /**
     * @param user The user to set.
     */
    public void setUser(User user) {
        m_user = user;
    }
}
