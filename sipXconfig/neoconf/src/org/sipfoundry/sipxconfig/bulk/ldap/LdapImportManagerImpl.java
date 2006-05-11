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
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.RowInserter;

public class LdapImportManagerImpl implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);

    private JndiLdapTemplate m_jndiTemplate;

    private RowInserter m_rowInserter;

    private AttrMap m_attrMap;

    public void insert() {
        String base = "dc=sipfoundry,dc=com";
        String filter = "objectclass=person";


        SearchControls sc = new SearchControls();
        sc.setSearchScope(SearchControls.SUBTREE_SCOPE);
        sc.setReturningAttributes(m_attrMap.getLdapAttributesArray());

        try {
            NamingEnumeration<SearchResult> result = m_jndiTemplate.search(base, filter, sc);
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                Attributes attributes = searchResult.getAttributes();
                m_rowInserter.execute(attributes);
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

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }
}
