/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends BeanWithGroups {

    public static final String GROUP_RESOURCE_ID = "user";
    
    // Reserved name for the special superadmin user.  In principle, this name could now be
    // anything, it's just "superadmin" by current convention.
    public static final String SUPERADMIN = "superadmin";

    private String m_firstName;

    private String m_sipPassword;

    private String m_pintoken;

    private String m_lastName;

    private String m_userName;

    private Set m_aliases = new LinkedHashSet(0);

    /**
     * Return the pintoken, which is the hash of the user's PIN. The PIN itself is private to the
     * user. To keep the PIN secure, we don't store it.
     */    
    public String getPintoken() {
        return (String) ObjectUtils.defaultIfNull(m_pintoken, StringUtils.EMPTY);
    }

    /**
     * Set the pintoken, which is the hash of the user's PIN. This method is only for prevent the
     * pintoken from being nulluse by Hibernate. Call setPin to change the PIN.
     */
    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    /**
     * Set the PIN, protecting it under a security realm. The PIN is private to the user. To keep
     * the PIN secure, we don't store it. Instead we store the "pintoken", which is a hash of the
     * PIN.
     * 
     * @param pin PIN
     * @param realm security realm
     */
    public void setPin(String pin, String realm) {
        String pin2 = (String) ObjectUtils.defaultIfNull(pin, StringUtils.EMPTY); // handle null
        // pin
        setPintoken(Md5Encoder.digestPassword(m_userName, realm, pin2));
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }

    public String getSipPasswordHash(String realm) {
        String password = (String) ObjectUtils.defaultIfNull(m_sipPassword, StringUtils.EMPTY);
        return Md5Encoder.digestPassword(m_userName, realm, password);
    }

    public void setSipPassword(String password) {
        m_sipPassword = password;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getUserName() {
        return (String) ObjectUtils.defaultIfNull(m_userName, StringUtils.EMPTY);
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getDisplayName() {
        Object[] names = {
            m_firstName, m_lastName
        };
        String s = StringUtils.join(names, ' ');
        return StringUtils.trimToNull(s);
    }

    public Set getAliases() {
        return m_aliases;
    }

    public void setAliases(Set aliases) {
        m_aliases = aliases;
    }

    /**
     * Copy the input aliases to become the aliases of this user, without replacing the Set
     * object. For a user read from the DB, Hibernate creates the Set and we don't want to mess
     * with it. Also, by copying the aliases, subsequent changes to the input Set won't affect the
     * user's Set, since it is a separate object.
     */
    public void copyAliases(Set aliases) {
        getAliases().clear();
        getAliases().addAll(aliases);
    }

    /**
     * Add the alias to the set of aliases.
     * Return true if the alias was added, false if the alias was already in the set.
     */
    public boolean addAlias(String alias) {
        return getAliases().add(alias);
    }

    public void addAliases(String[] aliases) {
        getAliases().addAll(Arrays.asList(aliases));
    }

    /** Return the aliases as a comma-delimited string */
    public String getAliasesString() {
        Set aliases = getAliases();
        return StringUtils.join(aliases.iterator(), ", ");
    }

    /** Set the aliases from a comma-delimited string */
    public void setAliasesString(String aliasesString) {
        String[] aliases = aliasesArrayFromString(aliasesString);
        getAliases().clear();
        addAliases(aliases);
    }

    /**
     * Given a comma-delimited string of aliases, return the aliases as a string array. Trim
     * leading and trailing whitespace from each alias.
     */
    public static String[] aliasesArrayFromString(String aliasesString) {
        if (StringUtils.isBlank(aliasesString)) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        String aliasesString2 = aliasesString.trim();
        String[] aliases = aliasesString2.split("\\s*,\\s*");
        return aliases;
    }
    
    /**
     * Given a comma-delimited string of group names, return the names as a string array. Trim
     * leading and trailing whitespace from each group name
     */
    public static String[] groupNamesFromString(String groupString) {
        return aliasesArrayFromString(groupString);
    }

    public String getUri(String domainName) {
        return SipUri.format(this, domainName);
    }

    public List getAliasMappings(String domainName) {
        final String contact = getUri(domainName);
        List mappings = new ArrayList(getAliases().size());
        for (Iterator iter = getAliases().iterator(); iter.hasNext();) {
            String alias = (String) iter.next();
            if (StringUtils.isBlank(alias)) {
                throw new RuntimeException("Found an empty alias for user " + m_userName);
            }
            final String identity = AliasMapping.createUri(alias, domainName);
            AliasMapping mapping = new AliasMapping(identity, contact);
            mappings.add(mapping);
        }

        return mappings;
    }

    /**
     * Check if a user has a specific permission
     */
    public boolean hasPermission(Permission permission) {
        Setting setting = getSettings().getSetting(permission.getSettingPath());
        if (setting == null) {
            throw new IllegalArgumentException("Setting " + permission.getName()
                    + " does not exist in user setting model");
        }
        boolean enabled = Permission.isEnabled(setting.getValue());
        return enabled;
    }
}
