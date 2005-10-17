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
package org.sipfoundry.sipxconfig.api;

import org.sipfoundry.sipxconfig.common.CoreContext;

public class TestServiceImpl implements TestService {
    
    private CoreContext m_coreContext;

    public void clearCoreContext() {
        m_coreContext.clear();
    }
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
