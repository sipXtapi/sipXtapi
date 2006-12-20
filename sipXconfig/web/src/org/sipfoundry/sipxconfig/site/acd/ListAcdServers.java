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
package org.sipfoundry.sipxconfig.site.acd;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.admin.commserver.ServerSettings;

public abstract class ListAcdServers extends BasePage implements PageBeginRenderListener {

    public abstract AcdContext getAcdContext();

    public abstract Collection getServers();

    public abstract void setServers(Collection servers);

    public void pageBeginRender(PageEvent event_) {
        if (getServers() == null) {
            setServers(getAcdContext().getServers());
        }
    }

    public IPage addServer(IRequestCycle cycle) {
        AcdServerPage editPage = (AcdServerPage) cycle.getPage(AcdServerPage.PAGE);
        editPage.setAcdServerId(null);
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage editServer(IRequestCycle cycle, Integer id) {
        AcdServerPage editPage = (AcdServerPage) cycle.getPage(AcdServerPage.PAGE);
        editPage.setAcdServerId(id);
        editPage.setReturnPage(this);
        return editPage;
    }

    public IPage activatePresenceServerPage(IRequestCycle cycle) {
        ServerSettings page = (ServerSettings) cycle.getPage(ServerSettings.PAGE);
        page.setParentSettingName("presence");
        return page;
    }

    public void formSubmit() {
        if (TapestryUtils.isValid(this)) {
            setServers(null);
        }
    }
}
