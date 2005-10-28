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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;

import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;

public class AddToPhoneGroupAction implements IActionListener, OptionAdapter {
    private PhoneContext m_phoneContext;
    private Collection m_ids;
    private Group m_group;

    public AddToPhoneGroupAction(Group group) {
        m_group = group;
    }

    public void actionTriggered(IComponent component_, IRequestCycle cycle_) {
        m_phoneContext.addToGroup(m_group.getId(), m_ids);
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setIds(Collection ids) {
        m_ids = ids;
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
        return "add_" + m_group.getId();
    }
}
