/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ClearFinishedJobsTag.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
 
package com.pingtel.pds.pgs.jsptags;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jobs.JobManager;
import com.pingtel.pds.pgs.jobs.JobManagerHome;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;

import javax.servlet.jsp.JspException;


public class ClearFinishedJobsTag extends ExTagSupport {

    private JobManager m_jobManagerEJBObject = null;

    public int doStartTag() throws JspException {
        try {
            if ( m_jobManagerEJBObject == null ) {
                JobManagerHome home = (JobManagerHome)
                        EJBHomeFactory.getInstance().getHomeInterface( JobManagerHome.class,
                                                                        "JobManager" );

                m_jobManagerEJBObject = home.create();
            }


            m_jobManagerEJBObject.flushJobs();
        }
        catch (Exception e ) {
            throw new JspException ( e.getMessage() );
        }

        return SKIP_BODY;
    }
}
