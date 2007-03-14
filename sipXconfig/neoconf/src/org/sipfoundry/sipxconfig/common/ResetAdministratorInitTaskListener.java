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

public class ResetAdministratorInitTaskListener extends InitTaskListener {
    private CoreContext m_coreContext;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    public void onInitTask(String task) {
        m_coreContext.createAdminGroupAndInitialUserTask();
    }
}
