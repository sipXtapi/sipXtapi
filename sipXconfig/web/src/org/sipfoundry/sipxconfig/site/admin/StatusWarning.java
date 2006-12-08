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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.job.JobContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class StatusWarning extends BaseComponent {
    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    /**
     * Show only if there was a failure AND we are NOT on JobStatus page
     * 
     * @return true if error should be showns
     */
    public boolean isShow() {
        return getJobContext().isFailure() && !JobStatusPage.PAGE.equals(getPage().getPageName());
    }
}
