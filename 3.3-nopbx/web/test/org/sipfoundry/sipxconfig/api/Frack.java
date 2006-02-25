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
public class Frack {
    private String m_name;
    private String m_abode;
    public Frack() {        
    }
    public Frack(String name, String abode) {
        setName(name);
        setAbode(abode);
    }
    public String getAbode() {
        return m_abode;
    }
    public void setAbode(String abode) {
        m_abode = abode;
    }
    public String getName() {
        return m_name;
    }
    public void setName(String name) {
        m_name = name;
    }
}
