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
package org.sipfoundry.sipxconfig.bulk.ldap;

/**
 * Maintains LDAP connection params, attribute maps and schedule
 * LdapManagerImpl
 */
public class LdapManagerImpl implements LdapManager {

    private AttrMap m_attrMap;
    private LdapConnectionParams m_connectionParams;

    public AttrMap getAttrMap() {
        return m_attrMap;
    }

    public LdapConnectionParams getConnectionParams() {
        return m_connectionParams;
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void setConnectionParams(LdapConnectionParams params) {
        m_connectionParams = params;
    }
}
