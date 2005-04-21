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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.site.user.UserTable;

public abstract class AddUserRing extends BasePage implements PageRenderListener {
    public static final String PAGE = "AddUserRing";

    public abstract CallGroupContext getCallGroupContext();

    public abstract CoreContext getCoreContext();

    public abstract Integer getCallGroupId();

    public abstract void setCallGroupId(Integer id);

    public abstract CallGroup getCallGroup();

    public abstract void setCallGroup(CallGroup callGroup);

    public void select(IRequestCycle cycle) {
        CallGroup callGroup = getCallGroup();

        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();

        for (Iterator i = selections.getAllSelected().iterator(); i.hasNext();) {
            Integer userId = (Integer) i.next();
            User user = getCoreContext().loadUser(userId.intValue());
            callGroup.insertRing(user);
        }

        getCallGroupContext().storeCallGroup(callGroup);
        returnToEditCallGroup(cycle);
    }

    public void cancel(IRequestCycle cycle) {
        returnToEditCallGroup(cycle);
    }

    public void pageBeginRender(PageEvent event_) {
        CallGroup callGroup = getCallGroupContext().loadCallGroup(getCallGroupId());
        setCallGroup(callGroup);
    }

    private void returnToEditCallGroup(IRequestCycle cycle) {
        EditCallGroup page = (EditCallGroup) cycle.getPage(EditCallGroup.PAGE);
        page.setCallGroupId(getCallGroupId());
        page.setCallGroup(null);
        cycle.activate(page);
    }
}
