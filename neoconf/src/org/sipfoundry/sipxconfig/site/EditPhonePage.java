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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Endpoint;

/**
 * Tapestry Page
 */
public class EditPhonePage extends BasePage {
    
    private Endpoint m_endpoint = new Endpoint();

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
    
    public void save(IRequestCycle cycleTemp) {
        // no-op avoid eclipse warning
        cycleTemp.getClass();
    }

    public void cancel(IRequestCycle cycleTemp) {
        // no-op avoid eclipse warning
        cycleTemp.getClass();
    }
}
