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

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl implements LdapManager {
    public static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);

    private AttrMap m_attrMap;

    private LdapConnectionParams m_connectionParams;

    private JndiLdapTemplate m_jndiTemplate;

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

    public void setJndiTemplate(JndiLdapTemplate jndiTemplate) {
        m_jndiTemplate = jndiTemplate;
    }

    public void verify(LdapConnectionParams params, AttrMap attrMap) {
        params.applyToTemplate(m_jndiTemplate);

        try {
            SearchControls cons = new SearchControls();
            cons.setReturningAttributes(new String[] {
                "namingContexts"
            });
            cons.setSearchScope(SearchControls.OBJECT_SCOPE);
            // ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
            NamingEnumeration<SearchResult> name = m_jndiTemplate.search("", "objectclass=*",
                    cons);
            LOG.info(name);

        } catch (NamingException e) {
            LOG.debug("Verifying LDAP connection", e);
            throw new UserException("Cannot connect to LDAP server: " + e.getMessage());
        }
    }
}
