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

import java.util.Timer;
import java.util.TimerTask;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.CronSchedule;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Maintains LDAP connection params, attribute maps and schedule LdapManagerImpl
 */
public class LdapManagerImpl implements LdapManager {
    public static final String FILTER_ALL_CLASSES = "objectclass=*";

    public static final Log LOG = LogFactory.getLog(LdapManagerImpl.class);

    private AttrMap m_attrMap;

    private LdapConnectionParams m_connectionParams;

    private JndiLdapTemplate m_jndiTemplate;

    private CronSchedule m_schedule = new CronSchedule();

    private LdapImportManager m_ldapImportManager;

    private Timer m_timer;

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

    public Schema getSchema() {
        getConnectionParams().applyToTemplate(m_jndiTemplate);
        try {
            return retrieveSchema();
        } catch (NamingException e) {
            LOG.debug("Retireving schema failed.", e);
            throw new UserException("Cannot retrieve schema from LDAP server: " + e.getMessage());
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
        NamingEnumeration<SearchResult> results = m_jndiTemplate.search("", FILTER_ALL_CLASSES,
                cons);
        // only interested in the first result
        if (results.hasMore()) {
            SearchResult result = results.next();
            return (String) result.getAttributes().get(attrs[0]).get();
        }
        return StringUtils.EMPTY;
    }

    private Schema retrieveSchema() throws NamingException {
        SearchControls cons = new SearchControls();
        String[] attrs = new String[] {
            "objectClasses"
        };

        cons.setReturningAttributes(attrs);
        cons.setSearchScope(SearchControls.OBJECT_SCOPE);
        NamingEnumeration<SearchResult> results = m_jndiTemplate.search("cn=subSchema",
                FILTER_ALL_CLASSES, cons);
        // only interested in the first result

        Schema schema = new Schema();
        if (!results.hasMore()) {
            return schema;
        }
        SearchResult result = results.next();
        NamingEnumeration definitions = result.getAttributes().get(attrs[0]).getAll();
        while (definitions.hasMoreElements()) {
            String classDefinition = (String) definitions.nextElement();
            schema.addClassDefinition(classDefinition);
        }
        return schema;
    }

    public CronSchedule getSchedule() {
        return m_schedule;
    }

    public void setSchedule(CronSchedule schedule) {
        if (m_timer != null) {
            m_timer.cancel();
        }
        m_schedule = schedule;
        m_timer = m_schedule.schedule(new LdapImportTask());
    }

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

    public void setLdapImportManager(LdapImportManager ldapImportManager) {
        m_ldapImportManager = ldapImportManager;
    }

    public void init() {
        m_timer = m_schedule.schedule(new LdapImportTask());
    }

    private final class LdapImportTask extends TimerTask {
        public void run() {
            m_ldapImportManager.insert();
        }
    }
}
