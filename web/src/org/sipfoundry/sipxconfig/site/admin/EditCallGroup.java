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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditCallGroup extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditCallGroup";

    public abstract CallGroupContext getCallGroupContext();

    public abstract Integer getCallGroupId();

    public abstract void setCallGroupId(Integer id);

    public abstract CallGroup getCallGroup();

    public abstract void setCallGroup(CallGroup callGroup);

    public void pageBeginRender(PageEvent event_) {
        CallGroup callGroup = getCallGroup();
        if (null != callGroup) {
            return;
        }
        Integer id = getCallGroupId();
        if (null != id) {
            CallGroupContext context = getCallGroupContext();
            callGroup = context.loadCallGroup(id);
        } else {
            callGroup = new CallGroup();
        }
        setCallGroup(callGroup);
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        // TODO; add optional validation here
        return !delegate.getHasErrors();
    }

    public void save(IRequestCycle cycle) {
        if (isValid()) {
            saveValid();
            cycle.activate(ListCallGroups.PAGE);
        }
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        CallGroup callGroup = getCallGroup();
        context.storeCallGroup(callGroup);
        Integer id = getCallGroup().getId();
        setCallGroupId(id);
    }
    
    public void formSubmit(IRequestCycle cycle_) {
        if (!isValid()) {
            return;
        }
        // TODO: add submit provessing
    }
    
}
