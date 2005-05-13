/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class CallGroup extends AbstractCallSequence {
    private boolean m_enabled;
    private String m_name;
    private String m_extension;
    private String m_lineName;
    private String m_description;

    public CallGroup() {
        // bean usage only
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getLineName() {
        return m_lineName;
    }

    public void setLineName(String lineName) {
        m_lineName = lineName;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    /**
     * inserts a new ring for a specific user
     * 
     * @param user
     * @return newly created user ring
     */
    public UserRing insertRing(User user) {
        UserRing ring = new UserRing();
        ring.setCallGroup(this);
        ring.setUser(user);
        insertRing(ring);
        return ring;
    }

    /**
     * Activates call group. Remove old lines that are associated with this group. Adds new lines
     * if group is enabled. Generates profiles and restarts all phones that have been affected by
     * the changes.
     */
    public void activate(PhoneContext phoneContext) {
        if (!isEnabled()) {
            return;
        }
        // for now - no line support implemented
        Set affectedPhones = addLines(new NoLine());
        phoneContext.generateProfilesAndRestart(affectedPhones);
    }

    /**
     * Create new lines for all devices associated with users in this call group.
     * 
     * @param phoneContext
     * @param domainName root organization domain name - to create lines URIs
     * @return collection of phones affected by the changes
     */
    private Set addLines(LineSupport lineSupport) {
        HashSet changedPhones = new HashSet();
        List calls = getCalls();
        for (Iterator i = calls.iterator(); i.hasNext();) {
            UserRing ur = (UserRing) i.next();
            User user = ur.getUser();
            changedPhones.addAll(lineSupport.createGroupLines(user));
        }
        return changedPhones;
    }

    /**
     * Create list of aliases that descibe forwarding for this group. All aliases have the form
     * group_name@domain => user_name@domain with the specific q value In addtion alias extension =>
     * group name is added to allow calling to extension
     * 
     * @return list of AliasMapping objects (identity => contact)
     */
    public List generateAliases(String domainName) {
        if (!isEnabled()) {
            return Collections.EMPTY_LIST;
        }
        String myIdentity = AliasMapping.createUri(m_name, domainName);

        List aliases = generateAliases(myIdentity, domainName);
        if (StringUtils.isNotBlank(m_extension)) {
            AliasMapping extensionAlias = new AliasMapping(AliasMapping.createUri(m_extension, domainName),
                    myIdentity);
            aliases.add(extensionAlias);
        }
        return aliases;
    }

    /**
     * Used to encapsulate the functionality related to creating additional lines required for
     * proper forwaring calls.
     * 
     * There are several strategies dealing with additional lines that we want to try without
     * changing the basic calling group functionality. LineSupport
     */
    interface LineSupport {
        /**
         * Create new lines for all devices associated with the in this call group.
         * 
         * @return collection of phones affected by the changes
         */
        Set createGroupLines(User user);
    }

    static class NoLine implements LineSupport {
        public Set createGroupLines(User user_) {
            return Collections.EMPTY_SET;
        }
    }

    static class DedicatedNoVmLine implements LineSupport {
        // private CoreContext m_coreContext;
        // private PhoneContext m_phoneContext;

        public Set createGroupLines(User user_) {
            // TODO: for each user add a dedicated line user(no_vm) on all phones that this user
            // is registered with

            return new HashSet();
        }
    }
}
