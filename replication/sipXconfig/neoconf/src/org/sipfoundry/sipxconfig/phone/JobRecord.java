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


/**
 * Represent a job record in 
 */
public class JobRecord {

    public static final int TYPE_PROJECTION = 0;

    public static final int TYPE_DEVICE_RESTART = 1;

    private int m_type;

    private Phone[] m_phones;    
    
    /**
     * Phone that job will operate on
     */
    public void setPhones(Phone[] phones) {
        m_phones = phones;
    }
    
    public Phone[] getPhones() {
        return m_phones;
    }    

    public int getType() {
        return m_type;
    }

    public void setType(int type) {
        m_type = type;
    }
}
