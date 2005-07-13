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
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.User;

public class CallGroup extends AbstractCallSequence {
    private boolean m_enabled;
    private String m_name;
    private String m_extension;
    private String m_description;

    public CallGroup() {
        // bean usage only
    }

    /**
     * We need a deep clone. Each call can only belong to single collection.
     */
    protected Object clone() throws CloneNotSupportedException {
        CallGroup clone = (CallGroup) super.clone();
        clone.clearRings();
        for (Iterator i = getCalls().iterator(); i.hasNext();) {
            UserRing ring = (UserRing) i.next();
            UserRing ringClone = (UserRing) ring.duplicate();
            ringClone.setCallGroup(clone);
            clone.insertRing(ringClone);
        }
        return clone;
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
            AliasMapping extensionAlias = new AliasMapping(AliasMapping.createUri(m_extension,
                    domainName), myIdentity);
            aliases.add(extensionAlias);
        }
        return aliases;
    }
}
