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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IActionListener;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.setting.Group;

public abstract class BulkGroupAction implements OptionAdapter, IActionListener {

    private Collection m_ids;

    private Group m_group;

    public BulkGroupAction(Group group) {
        m_group = group;
    }

    public Collection getIds() {
        return m_ids;
    }

    public void setIds(Collection ids) {
        m_ids = ids;
    }

    public Group getGroup() {
        return m_group;
    }

    public void setGroup(Group group) {
        m_group = group;
    }

    public Object getValue(Object option_, int index_) {
        return this;
    }

    public String getLabel(Object option_, int index_) {
        return m_group.getName();
    }

    public String squeezeOption(Object option_, int index_) {
        return getClass().getName() + m_group.getId();
    }

    @SuppressWarnings("unused")
    public String getSuccessMsg(Messages messages_) {
        // FIXME: use global message repository...
        // messages_.getMessage("msg.actionSuccess");
        return "Operation completed successfully";
    }
}
