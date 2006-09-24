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

import java.util.HashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Single holder of domain name
 */
public class Domain extends BeanWithId implements Cloneable {
    private String m_name;
    private Set<String> m_aliases;

    public Domain() {
    }

    public Domain(String name) {
        setName(name);
    }

    /**
     * Fully qualified host name is NOT using DNS SRV (e.g. myhost.example.com), otherwise use
     * domain name (example.com)
     */
    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }
    
    public boolean hasAliases() {
        return m_aliases != null && m_aliases.size() > 0;
    }

    /**
     * Addresses (potentially invalid or inaccessable to sipx) that sipx commservers will accept
     * messages for and treat as actual domain name.
     */
    public Set<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Set<String> aliases) {
        m_aliases = aliases;
    }

    public void addAlias(String alias) {
        if (m_aliases == null) {
            m_aliases = new HashSet();
        }
        m_aliases.add(alias);
    }

    public void removeAlias(String alias) {
        if (m_aliases != null) {
            m_aliases.remove(alias);
        }
    }
}
