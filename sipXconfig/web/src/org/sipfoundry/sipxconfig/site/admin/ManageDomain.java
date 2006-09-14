/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartReminder;

/**
 * Edit single domain and it's aliases
 */
public abstract class ManageDomain extends BasePage {
    public static final String PAGE = "ManageDomain";
    public abstract DomainManager getDomainManager();
    public abstract Domain getDomain();
    public abstract String getUneditedDomainName();
    
    public IPage commit(IRequestCycle cycle) {
        Domain d = getDomain();
        if (getUneditedDomainName().equals(d.getName())) {
            return null;
        }
        
        getDomainManager().saveDomain(d);            
        RestartReminder restartPage = (RestartReminder) cycle.getPage(RestartReminder.PAGE);
        restartPage.setNextPage(PAGE);
        return restartPage;
    }
}
