/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/jobs/JobManager.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jobs;

import javax.ejb.EJBObject;

public interface JobManager extends EJBObject, JobManagerBusiness {

    public static int PROJECTION = 0;
    public static int DEVICE_RESTART = 1;

    public static String STARTED = "S";
    public static String FAILED = "F";
    public static String COMPLETE = "C";

}
