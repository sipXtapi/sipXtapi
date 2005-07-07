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
package org.sipfoundry.sipxconfig.common;

import java.io.Serializable;

import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Ultimate parent for all objects. Represent the entire configuration for a particular company or
 * organization
 */
public class Organization implements Serializable {

    private static final long serialVersionUID = 1L;

    private Integer m_id = PhoneContext.UNSAVED_ID;
    
    private String m_name;
    
    private String m_dnsDomain;
    
    // TODO : Enumerate stereotypes
    private int m_stereotype = 1;
    
    /**
     * @return Returns the dnsDomain.
     */
    public String getDnsDomain() {
        return m_dnsDomain;
    }
    /**
     * @param dnsDomain The dnsDomain to set.
     */
    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain;
    }
    
    /**
     * @return Returns the stereoType.
     */
    public int getStereotype() {
        return m_stereotype;
    }
    /**
     * @param stereotype The stereoType to set.
     */
    public void setStereotype(int stereotype) {
        m_stereotype = stereotype;
    }
    
    /**
     * @return Returns the id.
     */
    public Integer getId() {
        return m_id;
    }
    /**
     * @param id The id to set.
     */
    public void setId(Integer id) {
        m_id = id;
    }
    /**
     * @return Returns the name.
     */
    public String getName() {
        return m_name;
    }
    /**
     * @param name The name to set.
     */
    public void setName(String name) {
        m_name = name;
    }

}
