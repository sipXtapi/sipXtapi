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

import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

/**
 * Edit single domain and it's aliases
 */
public abstract class ManageDomain extends BasePage {
    public abstract DomainManager getDomainManager();
    public abstract Domain getDomain();
    
    public void commit() {
        getDomainManager().saveDomain(getDomain());
    }
}
