/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/AssignDeviceToUserTag.java#4 $
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
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: </p>
 * @author unascribed
 * @version 1.0
 */

public class AssignDeviceToUserTag extends ExTagSupport {

    private String m_deviceID = null;
    private String m_userID = null;
    private UserAdvocateHome uaHome = null;
    private UserAdvocate uAdvocate = null;

    public void setUserid( String userid ){
        m_userID = userid;
    }

    public void setDeviceid( String deviceid ){
        m_deviceID = deviceid;
    }


    /**
     *
     * @return
     * @exception JspException
     */
    public int doStartTag() throws JspException {
        try {
            uaHome = (UserAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                    "UserAdvocate" );

            uAdvocate = uaHome.create();

            uAdvocate.assignDevice( m_deviceID, m_userID );

        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_deviceID = null;
        m_userID = null;

        super.clearProperties();
    }
}
