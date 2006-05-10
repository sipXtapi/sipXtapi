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

import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.DirContext;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.springframework.jndi.JndiAccessor;
import org.springframework.jndi.JndiCallback;
import org.springframework.jndi.JndiTemplate;

public class LdapImportManagerImpl implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);

    private JndiAccessor m_jndiAccessor;

    private RowInserter m_rowInserter;

    public void insert() {
        String[] attrs = {
            "uid"
        };
        String base = "dc=sipfoundry,dc=com";
        String filter = "objectclass=person";

        JndiTemplate jndiTemplate = m_jndiAccessor.getJndiTemplate();
        JndiCallback contextCallback = new Query(base, filter, attrs);
        try {
            NamingEnumeration<SearchResult> result = (NamingEnumeration<SearchResult>) jndiTemplate
                    .execute(contextCallback);
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                Attributes attributes = searchResult.getAttributes();
                m_rowInserter.execute(attributes);
            }

        } catch (NamingException e) {
            LOG.error("Retrieving users list from LDAP server", e);
        }
    }

    public void setJndiAccessor(JndiAccessor jndiAccessor) {
        m_jndiAccessor = jndiAccessor;
    }

    public void setRowInserter(RowInserter rowInserter) {
        m_rowInserter = rowInserter;
    }

    public static class Query implements JndiCallback {

        private String[] m_attrs;

        private String m_base;

        private String m_filter;

        public Query(String base, String filter, String[] attrs) {
            m_base = base;
            m_filter = filter;
            m_attrs = attrs;
        }

        public Object doInContext(Context ctx) throws NamingException {
            DirContext dirContext = (DirContext) ctx;

            SearchControls sc = new SearchControls();
            sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
            sc.setReturningAttributes(m_attrs);
            sc.setReturningObjFlag(true);

            return dirContext.search(m_base, m_filter, sc);
        }
    }

}
