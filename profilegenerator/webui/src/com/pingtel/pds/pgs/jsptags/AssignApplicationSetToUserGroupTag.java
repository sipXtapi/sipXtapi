/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/AssignApplicationSetToUserGroupTag.java#4 $
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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocate;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocateHome;
import org.apache.log4j.Category;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class AssignApplicationSetToUserGroupTag extends ExTagSupport {

    private String m_applicationGroupID;
    private String m_userGroupID;

    private Category logger = Category.getInstance( "pgs" );

    private ApplicationGroupAdvocateHome m_agaHome = null;

    public void setUsergroupid ( String usergroupid ) {
        m_userGroupID = usergroupid;
    }

    public void setApplicationsetid ( String applicationsetid ){
        m_applicationGroupID = applicationsetid;
    }


    public int doStartTag() throws JspException {
        try {
            ApplicationGroupAdvocate advocate = null;
            if ( m_agaHome == null ) {

                m_agaHome = (ApplicationGroupAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupAdvocateHome.class,
                                                                    "ApplicationGroupAdvocate" );
            }

            advocate = m_agaHome.create();

            logger.debug( "doing assign for user group: " +
                m_userGroupID + " app group: " + m_applicationGroupID );


            advocate.assignUserGroupAndApplicationGroup(   m_userGroupID,
                                                            m_applicationGroupID );
            logger.debug( "all done" );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
        m_applicationGroupID = null;
        m_userGroupID = null;

        super.clearProperties();
    }
}
