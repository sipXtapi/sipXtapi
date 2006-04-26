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
package org.sipfoundry.sipxconfig.job;

import org.apache.commons.lang.enums.Enum;

public final class JobStatus extends Enum {
    public static final JobStatus COMPLETED = new JobStatus("Completed");
    public static final JobStatus FAILED = new JobStatus("Failed");
    public static final JobStatus IN_PROGRESS = new JobStatus("In Progress");
    public static final JobStatus SCHEDULED = new JobStatus("Scheduled");

    private JobStatus(String name) {
        super(name);
    }
}
