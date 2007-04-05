/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

/**
 * Resets the superadmin user credentials to superadmin and pin to blank
 */
public class ResetAdministratorInitTaskListener implements SystemTaskEntryPoint {
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void runSystemTask(String[] args) {
        m_coreContext.createAdminGroupAndInitialUserTask();
    }
}
