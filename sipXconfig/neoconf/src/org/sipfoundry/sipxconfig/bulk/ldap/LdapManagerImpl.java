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

import org.apache.commons.lang.StringUtils;
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
            String searchBase = retrieveDefaultSearchBase();
            // it will only overwrite the search base if not set yet
            if (StringUtils.isBlank(attrMap.getSearchBase())) {
                attrMap.setSearchBase(searchBase);
            }
        } catch (NamingException e) {
            LOG.debug("Verifying LDAP connection failed.", e);
            throw new UserException("Cannot connect to LDAP server: " + e.getMessage());
        }
    }

    /**
     * Connects to LDAP to retrieve the namingContexts attribute from root. Good way to verify if
     * LDAP is accessible. Command line anologue is:
     * 
     * ldapsearch -x -b '' -s base '(objectclass=*)' namingContexts
     * 
     * @return namingContext value - can be used as the search base for user if nothing more
     *         specific is provided
     * @throws NamingException
     */
    private String retrieveDefaultSearchBase() throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "namingContexts"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        NamingEnumeration<SearchResult> results = m_jndiTemplate
                .search("", "objectclass=*", cons);
        // only interested in the first result
        if (results.hasMore()) {
            SearchResult result = results.next();
            return (String) result.getAttributes().get(attrs[0]).get();
        }
        return StringUtils.EMPTY;
    }
}
