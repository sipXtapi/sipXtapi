/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

/** sample test bean */
public class Frick {    
    private String m_name;
    private String m_address;
    public Frick() {        
    }
    public Frick(String name, String address) {
        setName(name);
        setAddress(address);
    }
    public String getAddress() {
        return m_address;
    }
    public void setAddress(String address) {
        m_address = address;
    }
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }
}
