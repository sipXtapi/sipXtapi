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
package org.sipfoundry.sipxconfig.vendor;

import org.sipfoundry.sipxconfig.core.Endpoint;
import org.sipfoundry.sipxconfig.core.Phone;

/**
 * Support for Polycom 300, 400, and 500 series phones and model 3000
 * conference phone
 */
public class PolycomPhone implements Phone {
    
    /** basic model */
    public static final String MODEL_300 = "polycom300";

    /** standard model */
    public static final String MODEL_500 = "polycom500";

    /** deluxe model */
    public static final String MODEL_600 = "polycom600";

    /** conference phone */
    public static final String MODEL_3000 = "polycom3000";
    
    private String m_id;
    
    private Endpoint m_endpoint;
    
    public String getModelId() {
        return m_id;
    }
    
    public void setModel(String id) {
        m_id = id;
    }

    public String generateProfile(int profileIndex) {
        return "Profile #" + profileIndex;
    }

    public int getProfileCount() {
        return 1;
    }
    
    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
   
}
