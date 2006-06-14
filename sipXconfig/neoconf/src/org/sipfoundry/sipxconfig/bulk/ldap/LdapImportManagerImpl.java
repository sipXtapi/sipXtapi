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

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class LdapImportManagerImpl extends HibernateDaoSupport implements LdapImportManager {
    public static final Log LOG = LogFactory.getLog(LdapImportManager.class);

    private JndiLdapTemplate m_jndiTemplate;

    private LdapManager m_ldapManager;

    private LdapRowInserter m_rowInserter;

    private int m_previewSize;

    public void insert() {
        try {
            NamingEnumeration<SearchResult> result = search(0);
            m_rowInserter.beforeInserting();
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                m_rowInserter.execute(searchResult);
            }
            m_rowInserter.afterInserting();

        } catch (NamingException e) {
            LOG.error("Retrieving users list from LDAP server", e);
        }
    }

    public List<UserPreview> getExample() {
        try {
            ArrayList<UserPreview> example = new ArrayList<UserPreview>(m_previewSize);
            NamingEnumeration<SearchResult> result = search(m_previewSize);
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                User user = new User();
                m_rowInserter.setUserProperties(user, searchResult.getAttributes());
                List<String> groupNames = new ArrayList<String>(m_rowInserter
                        .getGroupNames(searchResult));
                example.add(new UserPreview(user, groupNames));
            }
            return example;
        } catch (NamingException e) {
            throw new UserException(e.getMessage());
        }
    }

    public void dumpExample(Writer out) {
        try {
            CsvWriter writer = new CsvWriter(out);
            String[] allNames = Index.getAllNames();
            writer.write(allNames, false);
            
            NamingEnumeration<SearchResult> result = search(0);
            while (result.hasMore()) {
                SearchResult searchResult = result.next();
                User user = new User();
                m_rowInserter.setUserProperties(user, searchResult.getAttributes());
                List<String> groupNames = new ArrayList<String>(m_rowInserter
                        .getGroupNames(searchResult));
                String groupNamesString = StringUtils.join(groupNames.iterator(), ", ");
                String[] row = new String[allNames.length];
                Index.USERNAME.set(row, user.getUserName());
                Index.FIRST_NAME.set(row, user.getFirstName());
                Index.LAST_NAME.set(row, user.getLastName());
                Index.ALIAS.set(row, user.getAliasesString());
                Index.SIP_PASSWORD.set(row, user.getSipPassword());
                Index.USER_GROUP.set(row, groupNamesString);                
                
                writer.write(row, true);
            }
        } catch (NamingException e) {
            throw new UserException(e.getMessage());
        } catch (IOException e) {
            throw new UserException(e.getMessage());
        }
    }

    public void setJndiTemplate(JndiLdapTemplate jndiTemplate) {
        m_jndiTemplate = jndiTemplate;
    }

    public void setRowInserter(LdapRowInserter rowInserter) {
        m_rowInserter = rowInserter;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setPreviewSize(int previewSize) {
        m_previewSize = previewSize;
    }

    private NamingEnumeration<SearchResult> search(long limit) throws NamingException {
        SearchControls sc = new SearchControls();
        sc.setCountLimit(limit);
        sc.setSearchScope(SearchControls.SUBTREE_SCOPE);

        AttrMap attrMap = m_ldapManager.getAttrMap();
        if (!attrMap.verified()) {
            m_ldapManager.verify(m_ldapManager.getConnectionParams(), attrMap);
        }

        sc.setReturningAttributes(attrMap.getLdapAttributesArray());

        String base = attrMap.getSearchBase();
        String filter = attrMap.getSearchFilter();

        // FIXME: this is a potential threading problem - we cannot have one template shared
        // if we are changing the connection params for each insert operation
        m_ldapManager.getConnectionParams().applyToTemplate(m_jndiTemplate);
        m_rowInserter.setAttrMap(attrMap);
        NamingEnumeration<SearchResult> result = m_jndiTemplate.search(base, filter, sc);
        return result;
    }

}
