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
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.setting.Setting;


/**
 * Polycom business functions for line meta setting
 */
public class PolycomEndpoint {
    
    public static final String SERVER = "server";
    
    public static final String ADDRESS = "address";

    public static final String FIRST = "1";
    
    private PolycomPhone m_phone;
    
    private Setting m_settings;
    
    private Endpoint m_endpoint;
    
    public PolycomEndpoint(PolycomPhone phone, Endpoint endpoint) {
        m_phone = phone;
        m_endpoint = endpoint;
        m_settings = m_phone.getEndpointModel().copy();
        setDefaults();

        // hack, avoid unused warning
        m_endpoint.getClass();
    }
    
    private void setDefaults() {        
        String domainName = m_phone.getCoreContext().loadRootOrganization().getDnsDomain();
        Setting voip = m_settings.getSetting("voIpProt");
        voip.getSetting(SERVER).getSetting(FIRST).getSetting(ADDRESS).setValue(domainName);
        voip.getSetting("SIP.outboundProxy").getSetting(ADDRESS).setValue(domainName);
    }
    
    public Setting getSettings() {
        return m_settings;
    }
}
