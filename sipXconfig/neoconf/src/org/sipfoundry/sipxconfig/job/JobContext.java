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

import java.io.Serializable;
import java.util.List;

public interface JobContext {
    Serializable schedule(String name);

    void start(Serializable jobId);

    void success(Serializable jobId);

    void failure(Serializable jobId, String errorMsg, Throwable exception);

    int removeCompleted();

    void clear();

    List<Job> getJobs();

    /**
     * Returns true if there is a least one failed job on the list.
     */
    boolean isFailure();
}
