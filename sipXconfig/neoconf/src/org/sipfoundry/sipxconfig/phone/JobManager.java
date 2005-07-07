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
package org.sipfoundry.sipxconfig.phone;


/**
 * Run an ansynchorous operations on job records in JobQueue
 * Setting the JobQueue start the processing.  
 */
public interface JobManager {
    
    public void stopProcessingJobRecords();
    
    public void finishProcessingJobs();
}
