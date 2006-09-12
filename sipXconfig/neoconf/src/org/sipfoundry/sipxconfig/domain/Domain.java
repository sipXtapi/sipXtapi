/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Set;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Single holder of domain name 
 */
public class Domain extends BeanWithId {
    private String m_name;
    private Set<String> m_aliases;

    /**
     * Fully qualified host name is NOT using DNS SRV (e.g. myhost.example.com),
     * otherwise use domain name (example.com)  
     * @return
     */
    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    /**
     * Addresses (potentially invalid or inaccessable to sipx) that sipx commservers
     * will accept messages for and treat as actual domain name.
     * 
     * @return
     */
    public Set<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Set<String> aliases) {
        m_aliases = aliases;
    }
    
    public String getLocalFQDN() {
        return "localhost.localdomain";
    }
}
