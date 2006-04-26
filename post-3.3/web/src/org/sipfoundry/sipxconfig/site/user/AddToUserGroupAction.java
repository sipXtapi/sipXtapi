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
package org.sipfoundry.sipxconfig.site.user;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

public class AddToUserGroupAction extends BulkGroupAction {
    private CoreContext m_coreContext;

    public AddToUserGroupAction(Group group, CoreContext coreContext) {
        super(group);
        m_coreContext = coreContext;
    }

    public void actionTriggered(IComponent component_, IRequestCycle cycle_) {
        m_coreContext.addToGroup(getGroup().getId(), getIds());
    }

    public String getSuccessMsg(Messages messages) {
        return messages.format("msg.success.addToUserGroupAction", Integer.toString(getIds()
                .size()), getGroup().getName());
    }
}
