/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/JobStatusTag.java#5 $
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

public class JobStatusTag extends StyleTagSupport {

    String m_jobTypes = null;

    private JobManager m_jobManagerEJBObject = null;

    public void setJobtypes ( String jobtypes ) {
        m_jobTypes = jobtypes;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_jobManagerEJBObject == null ) {
                JobManagerHome home = (JobManagerHome)
                        EJBHomeFactory.getInstance().getHomeInterface( JobManagerHome.class,
                                                                        "JobManager" );

                m_jobManagerEJBObject = home.create();
            }

            Element statuses = null;

            if ( m_jobTypes.equalsIgnoreCase ( "all" ) ) {
                statuses = m_jobManagerEJBObject.getAllJobStatuses();
            }
            else {
                statuses = m_jobManagerEJBObject.getCurrentJobStatuses();
            }

            outputTextToBrowser ( statuses );
        }
        catch (Exception e ) {
            throw new JspException ( e.getMessage() );
        }


        return SKIP_BODY;
    }


    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {
        // @JC TODO
        return inputElement;
    }


    protected void clearProperties() {
        super.clearProperties();

        m_jobTypes = null;
    }


}
