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

import java.lang.reflect.InvocationTargetException;
import java.util.Set;
import java.util.TreeSet;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import org.apache.commons.beanutils.BeanUtils;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.bulk.csv.CsvRowInserter.Index;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Specialized version of row inserter for inserting users from LDAP searches
 * LdapRowinserter
 */
public class LdapRowinserter extends RowInserter<SearchResult> {

    private AttrMap m_attrMap;

    private CoreContext m_coreContext;

    /**
     * Initial implementation will just print all attributes...
     */
    protected String dataToString(SearchResult sr) {
        return sr.getName();
    }

    protected void insertRow(SearchResult sr) {
        try {
            Attributes attrs = sr.getAttributes();
            LOG.info("Inserting:" + attrs.toString());
            String attrName = m_attrMap.getIdentityAttributeName();
            String userName;
            userName = getValue(attrs, attrName);

            User user = m_coreContext.loadUserByUserName(userName);
            String defaultPin = null;
            if (user == null) {
                user = new User();
                user.setUserName(userName);
                defaultPin = m_attrMap.getDefaultPin();
            }

            setProperty(user, attrs, Index.FIRST_NAME);
            setProperty(user, attrs, Index.LAST_NAME);
            setProperty(user, attrs, Index.SIP_PASSWORD);

            Set<String> aliases = getValues(attrs, Index.ALIAS);
            user.setAliases(aliases);

            String pin = getValue(attrs, Index.PIN);
            if (pin == null) {
                pin = defaultPin;
            }
            if (pin != null) {
                user.setPin(pin, m_coreContext.getAuthorizationRealm());
            }

            m_coreContext.saveUser(user);
        } catch (NamingException e) {
            throw new RuntimeException(e);
        }
    }

    private void setProperty(User user, Attributes attrs, Index index) throws NamingException {
        try {
            String value = getValue(attrs, index);
            if (value != null) {
                BeanUtils.setProperty(user, index.getName(), value);
            }
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e.getCause());
        }
    }

    private String getValue(Attributes attrs, Index index) throws NamingException {
        String attrName = m_attrMap.userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValue(attrs, attrName);
    }

    private Set<String> getValues(Attributes attrs, Index index) throws NamingException {
        String attrName = m_attrMap.userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValues(attrs, attrName);
    }

    /**
     * Returns single value for an attribute, even if attribute has more values...
     * 
     * @param attrs collection of attributes
     * @param attr attribute name
     */
    private String getValue(Attributes attrs, String attrName) throws NamingException {
        Attribute attribute = attrs.get(attrName);
        if (attribute == null) {
            return null;
        }
        return (String) attribute.get();
    }

    /**
     * Returns all string values for an attribute with a given name, ignores the values that are
     * not string values
     * 
     * @param attrs collection of attributes
     * @param attr attribute name
     */
    private Set<String> getValues(Attributes attrs, String attrName) throws NamingException {
        Attribute attribute = attrs.get(attrName);
        if (attribute == null) {
            return null;
        }
        Set<String> values = new TreeSet<String>();
        NamingEnumeration< ? > allValues = attribute.getAll();
        while (allValues.hasMore()) {
            Object object = allValues.nextElement();
            if (object instanceof String) {
                values.add((String) object);
            }
        }
        return values;
    }

    protected boolean checkRowData(SearchResult sr) {
        Attributes attrs = sr.getAttributes();
        String idAttrName = m_attrMap.getIdentityAttributeName();
        return (attrs.get(idAttrName) != null);
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
