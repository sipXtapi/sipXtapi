/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.legacy.LegacyContext;
import org.sipfoundry.sipxconfig.legacy.UserConfigSet;

public abstract class EditSipPassword extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditSipPassword";
    
    public abstract User getUser();
    
    public abstract void setUser(User user);
    
    public abstract Integer getUserId();
    
    public abstract void setUserId(Integer userId);

    public abstract void setLegacyContext(LegacyContext legacy);

    public abstract LegacyContext getLegacyContext();

    public abstract void setCoreContext(CoreContext legacy);

    public abstract CoreContext getCoreContext();

    public abstract void setSipPassword(String password);

    public abstract String getSipPassword();
    
    public void save(IRequestCycle cycle) {
        User user = getUser();
        UserConfigSet cfg = getLegacyContext().getConfigSetForUser(user);
        String passtoken = Md5Encoder.digestPassword(user.getDisplayId(), 
                getCoreContext().getAuthorizationRealm(), getSipPassword());
        cfg.setSipPassword(getSipPassword(), passtoken);
        getLegacyContext().updateConfigSet(cfg);
        getLegacyContext().triggerCredentialGeneration();
        cycle.activate(ManageUsers.PAGE);
    }
    
    public void pageBeginRender(PageEvent event_) {
        // FIXME: Why is user id int, and not Integer?
        User user = getCoreContext().loadUser(getUserId().intValue());
        setUser(user);
        
        UserConfigSet cfg = getLegacyContext().getConfigSetForUser(getUser());
        setSipPassword(cfg.getClearTextPassword());        
    }
}
