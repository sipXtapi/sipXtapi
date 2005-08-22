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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.Server;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class ServerSettings extends BasePage implements PageRenderListener {
    public static final String PAGE = "ServerSettings";

    public abstract Server getServer();

    public abstract String getParentSettingName();

    public abstract void setParentSettingName(String name);

    public abstract Setting getParentSetting();

    public abstract void setParentSetting(Setting parent);
    
    public abstract CoreContext getCoreContext();

    public abstract PhoneContext getPhoneContext();
    
    public abstract SipxReplicationContext getSipxReplicationContext();

    public void pageBeginRender(PageEvent event_) {
        Server server = getServer();
        Setting root = server.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    /*
     * private boolean isValid() { IValidationDelegate delegate =
     * TapestryUtils.getValidator(this); return !delegate.getHasErrors(); }
     * 
     * public void formSubmit(IRequestCycle cycle_) { if (!isValid()) { return; }
     * getServer().applySettings(); }
     */

    public void apply(IRequestCycle cycle) {
        getServer().applySettings();
        handlePossibleDomainNameChange();
        RestartReminder restartPage = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        restartPage.setNextPage(PAGE);
        cycle.activate(restartPage);
    }
    
    private void handlePossibleDomainNameChange() {
        String oldDomainName = getCoreContext().getDomainName();
        String newDomainName = getServer().getServerSettings().getDomainName();

        // bail if domain name wasn't changed.
        if (newDomainName.equals(oldDomainName)) {
            return;
        }
        
        // propagate domain name to all known copies.  Hopefully better
        // way will surface when server settings aren't assumed to be
        // local
        getCoreContext().setDomainName(newDomainName);
        getPhoneContext().getPhoneDefaults().setDomainName(newDomainName);

        // hopefully happens before user restarts servers.
        getSipxReplicationContext().generateAll();        
    }

    public void cancel(IRequestCycle cycle_) {
        // do nothing
    }
}
