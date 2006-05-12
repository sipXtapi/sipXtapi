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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.functors.NotNullPredicate;

/**
 * Information related to mapping LDAP attributes to User properties
 */
public class AttrMap {
    private Map<String, String> m_user2ldap = new TreeMap<String, String>();

    /**
     * name of the group that will keep all users imported from LDAP
     */
    private String m_defaultGroupName;

    /**
     * LDAP attribute name for an attribute matched to username
     */
    private String m_identityAttributeName;

    /**
     * PIN to be used for Users that do not have PIN mapped
     */
    private String m_defaultPin;

    /**
     * Returns non null LDAP attributes. Used to limit search results.
     */
    public Collection<String> getLdapAttributes() {
        Collection<String> attrs = new ArrayList<String>(m_user2ldap.values());
        CollectionUtils.filter(attrs, NotNullPredicate.INSTANCE);
        return attrs;
    }

    public String[] getLdapAttributesArray() {
        Collection<String> attrs = getLdapAttributes();
        return attrs.toArray(new String[attrs.size()]);
    }

    public String userProperty2ldapAttribute(String propertyName) {
        return m_user2ldap.get(propertyName);
    }

    public void setDefaultPin(String defaultPin) {
        m_defaultPin = defaultPin;
    }

    public void setIdentityAttributeName(String identityAttributeName) {
        m_identityAttributeName = identityAttributeName;
    }

    public String getIdentityAttributeName() {
        return m_identityAttributeName;
    }

    public String getDefaultPin() {
        return m_defaultPin;
    }

    public void setDefaultGroupName(String defaultGroupName) {
        m_defaultGroupName = defaultGroupName;
    }

    public String getDefaultGroupName() {
        return m_defaultGroupName;
    }

    public void setUserToLdap(Map<String, String> user2ldap) {
        m_user2ldap = user2ldap;
    }
}
