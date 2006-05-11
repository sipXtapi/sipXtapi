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
import java.util.Arrays;
import java.util.Collection;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.functors.NotNullPredicate;
import org.apache.commons.lang.ArrayUtils;

public class AttrMap {
    private final String[] m_userProperties = {
        "userName", "pin", "sipPassword", "firstName", "lastName", "aliasesString",
        "userGroupName"
    };

    private final String[] m_ldapAttributes = {
        "uid", null, null, "givenName", "sn", "telephoneNumber", null
    };

    public Collection<String> getLdapAttributes() {
        Collection<String> attrs = new ArrayList<String>(Arrays.asList(m_ldapAttributes));
        CollectionUtils.filter(attrs, NotNullPredicate.INSTANCE);
        return attrs;
    }

    public String[] getLdapAttributesArray() {
        Collection<String> attrs = getLdapAttributes();
        return attrs.toArray(new String[attrs.size()]);
    }

    public String ldapAttr2userProperty(String attr) {
        int i = ArrayUtils.indexOf(m_ldapAttributes, attr);
        if (i < 0) {
            return null;
        }
        return m_userProperties[i];
    }

    /**
     * test only
     */
    public boolean invariant() {
        return m_userProperties.length == m_ldapAttributes.length;
    }
}
