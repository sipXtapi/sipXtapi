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
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class LdapImportManagerImpl extends HibernateDaoSupport implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);

    private JndiLdapTemplate m_jndiTemplate;

    private LdapManager m_ldapManager;

    private RowInserter< ? extends SearchResult> m_rowInserter;

    public void insert() {
        String base = "dc=sipfoundry,dc=com";
        String filter = "objectclass=person";

        // FIXME: this is a potential threading problem - we cannot have one template shared if we
        // are changing the connection params for each insert operation
        m_ldapManager.getConnectionParams().applyToTemplate(m_jndiTemplate);

        SearchControls sc = new SearchControls();
        sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
        sc.setReturningAttributes(m_ldapManager.getAttrMap().getLdapAttributesArray());

        try {
            NamingEnumeration<SearchResult> result = m_jndiTemplate.search(base, filter, sc);
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                m_rowInserter.execute(searchResult);
            }

        } catch (NamingException e) {
            LOG.error("Retrieving users list from LDAP server", e);
        }
    }

    public void setJndiTemplate(JndiLdapTemplate jndiTemplate) {
        m_jndiTemplate = jndiTemplate;
    }

    public void setRowInserter(RowInserter rowInserter) {
        m_rowInserter = rowInserter;
    }
}
