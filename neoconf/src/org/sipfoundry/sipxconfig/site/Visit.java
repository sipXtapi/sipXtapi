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

import java.io.Serializable;

/**
 * Gets serialized for each user into HttpSession.
 */
public class Visit implements Serializable {

    private static final long serialVersionUID;
    
    static {
        serialVersionUID = 1L;
    }

    private String m_foobar;

    /**
     * don't know what I'm doing yet w/tapestry
     * 
     * @param foobar
     */
    public void setFoobar(String foobar) {
        m_foobar = foobar;
    }

    public String getFoobar() {
        return m_foobar;
    }
}