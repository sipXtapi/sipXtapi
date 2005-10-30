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

import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.phone.PhoneManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public class RemoveFromPhoneGroupAction extends BulkGroupAction {
    private PhoneManager m_phoneContext;

    public RemoveFromPhoneGroupAction(Group group, PhoneManager phoneContext) {
        super(group);
        m_phoneContext = phoneContext;
    }

    public void actionTriggered(IComponent component_, IRequestCycle cycle_) {
        m_phoneContext.removeFromGroup(getGroup().getId(), getIds());
    }

    public void setPhoneContext(PhoneManager phoneContext) {
        m_phoneContext = phoneContext;
    }
}
