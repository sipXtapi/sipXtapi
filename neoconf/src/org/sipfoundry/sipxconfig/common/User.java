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

    public static final String USER_NAME_PROPERTY = "userName"; 
    public static final String USER_NAME_OBJECT_PROPERTY = "userNameObject"; 
    public static final String FIRST_NAME_PROPERTY = "firstName"; 
    public static final String LAST_NAME_PROPERTY = "lastName";
    
    private String m_firstName;
    private String m_sipPassword;
    private String m_pintoken;
    private String m_lastName;
    private UserName m_userNameObject;
    private Set m_aliasObjects = new LinkedHashSet(0);
    
    /**
     * Return the pintoken, which is the hash of the user's PIN. The PIN itself is private to the
     * user. To keep the PIN secure, we don't store it.
     */
    public String getPintoken() {
        return (String) ObjectUtils.defaultIfNull(m_pintoken, StringUtils.EMPTY);
    }

    /**
     * Set the pintoken, which is the hash of the user's PIN. This method is only for 
     * use by Hibernate. Call setPin to change the PIN.
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
        setPintoken(Md5Encoder.digestPassword(getUserName(), realm, pin2));
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
        return Md5Encoder.digestPassword(getUserName(), realm, password);
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

    public UserName getUserNameObject() {
        return m_userNameObject;
    }

    public void setUserNameObject(UserName userNameObject) {
        m_userNameObject = userNameObject;
    }

    public String getUserName() {
        if (m_userNameObject == null) {
            return StringUtils.EMPTY;
        }
        return (String) ObjectUtils.defaultIfNull(m_userNameObject.getName(), StringUtils.EMPTY);
    }

    public void setUserName(String userName) {
        if (m_userNameObject == null) {
            m_userNameObject = new UserName(userName);
        } else {
            m_userNameObject.setName(userName);
        }
    }

    public String getDisplayName() {
        Object[] names = {
            m_firstName, m_lastName
        };
        String s = StringUtils.join(names, ' ');
        return StringUtils.trimToNull(s);
    }

    public Set getAliasObjects() {
        return m_aliasObjects;
    }

    public void setAliasObjects(Set aliasObjects) {
        m_aliasObjects = aliasObjects;
    }
    
    /** 
     * Return all names for the user, including both the userName and aliases.
     * The userName is the first array entry.
     */
    public String[] getNames() {
        int numNames = getAliasObjects().size() + 1;
        String[] names = new String[numNames];
        copyAliasesToArray(names, 1);
        names[0] = getUserName();
        return names;
    }

    public String[] getAliases() {
        String[] aliases = new String[getAliasObjects().size()];
        copyAliasesToArray(aliases, 0);
        return aliases;
    }

    private void copyAliasesToArray(String[] array, int offset) {
        int count = 0;
        for (Iterator iter = getAliasObjects().iterator(); iter.hasNext();) {
            UserName un = (UserName) iter.next();
            array[count++ + offset] = un.getName();
        }        
    }
    
    public void setAliases(String[] aliases) {
        getAliasObjects().clear();
        addAliases(aliases);
    }

    /**
     * Add the alias to the set of aliases.
     * Return true if the alias was added, false if the alias was already in the set.
     */
    public boolean addAlias(String alias) {
        boolean doAdd = !hasAlias(alias);
        if (doAdd) {
            getAliasObjects().add(new UserName(alias));
        }
        return doAdd;
    }

    public void addAliases(String[] aliases) {
        if (aliases == null) {
            return;
        }
        for (int i = 0; i < aliases.length; i++) {
            addAlias(aliases[i]);
        }
    }
    
    public void removeAlias(String alias) {
        for (Iterator iter = getAliasObjects().iterator(); iter.hasNext();) {
            UserName un = (UserName) iter.next();
            if (un.getName().equals(alias)) {
                getAliasObjects().remove(un);
            }
        }
    }

    /** Return true iff alias is one of the aliases for this user */
    public boolean hasAlias(String alias) {
        // We expect users to have very few aliases.  Therefore simple linear search is fine.
        // Loop through the aliases and see if the input alias matches one of them.
        for (Iterator iter = getAliasObjects().iterator(); iter.hasNext();) {
            UserName un = (UserName) iter.next();
            if (un.getName().equals(alias)) {
                return true;
            }
        }
        return false;
    }
    
    /** Return the aliases as a comma-delimited string */
    public String getAliasesString() {
        String[] aliases = getAliases();
        return StringUtils.join(aliases, ", ");
    }

    /** Set the aliases from a comma-delimited string */
    public void setAliasesString(String aliasesString) {
        String[] aliases = aliasesArrayFromString(aliasesString);
        setAliases(null);
        addAliases(aliases);
    }

    /**
     * Given a comma-delimited string of aliases, return the aliases as a string array.
     * Trim leading and trailing whitespace from each alias.
     */
    public static String[] aliasesArrayFromString(String aliasesString) {
        if (StringUtils.isBlank(aliasesString)) {
            return ArrayUtils.EMPTY_STRING_ARRAY;
        }
        String aliasesString2 = aliasesString.trim();
        String[] aliases = aliasesString2.split("\\s*,\\s*");
        return aliases;
    }

    public String getUri(String domainName) {
        return SipUri.format(this, domainName);
    }

    public List getAliasMappings(String domainName) {
        final String contact = getUri(domainName);
        String[] aliases = getAliases();
        List mappings = new ArrayList(aliases.length);
        for (int i = 0; i < aliases.length; i++) {
            String alias = aliases[i];
            if (StringUtils.isBlank(alias)) {
                throw new RuntimeException("Found an empty alias for user " + getUserName());
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
