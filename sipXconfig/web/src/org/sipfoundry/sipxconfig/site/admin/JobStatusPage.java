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
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.job.JobContext;

public abstract class JobStatusPage extends BasePage {
    public abstract JobContext getJobContext();
    
    /** No-op function does nothing. It is used as the listener function for the Refresh button. */
    public void noop(IRequestCycle cycle_) {
    }

    public void remove(IRequestCycle cycle_) {
        getJobContext().removeCompleted();
    }
}
