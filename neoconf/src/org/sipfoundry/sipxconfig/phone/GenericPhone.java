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

import java.io.IOException;

/**
 * Implements some of the more menial methods of Phone interface 
 */
public abstract class GenericPhone implements Phone {

    public static final String GENERIC_PHONE_ID = "generic";

    private String m_id = GENERIC_PHONE_ID;

    private String m_systemDirectory;
    
    public String getModelId() {
        return m_id;
    }

    public void setModelId(String id) {
        m_id = id;
    }

    public void restart(Endpoint endpoint_) throws IOException {
    }

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }
}
