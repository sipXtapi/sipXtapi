/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/GetJobExceptionMessageTag.java#5 $
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
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import org.jdom.Element;

import javax.servlet.jsp.JspException;


public class GetJobExceptionMessageTag extends StyleTagSupport {

    String m_jobID = null;

    private JobManager m_jobManagerEJBObject = null;

    public void setJobid ( String jobid ) {
        m_jobID = jobid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_jobManagerEJBObject == null ) {
                JobManagerHome home = (JobManagerHome)
                        EJBHomeFactory.getInstance().getHomeInterface( JobManagerHome.class,
                                                                        "JobManager" );

                m_jobManagerEJBObject = home.create();
            }

            Element statuses =
                m_jobManagerEJBObject.getExceptionMessage( new Integer ( m_jobID ).intValue() );

            outputTextToBrowser ( statuses );
        }
        catch (Exception e ) {
            throw new JspException ( e.getMessage() );
        }

        return SKIP_BODY;
    }


    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {

        return inputElement;
    }


    protected void clearProperties() {
        super.clearProperties();

        m_jobID = null;
    }
}
