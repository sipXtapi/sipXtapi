/*
 * 
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
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.HashMap;



public class ProjectProfilesTag extends JMSTagHandler {

    private DeviceAdvocateHome m_deviceAdvocateHome = null;
    private UserAdvocateHome m_userAdvocateHome = null;

    private String m_entityType = null;
    private String m_entityID = null;
    private String m_profileTypes = null;

    public void setEntitytype ( String entitytype ) {
        this.m_entityType = entitytype;
    }

    public void setEntityid ( String entityid ) {
        this.m_entityID = entityid;
    }

    public void setProfiletypes ( String profiletypes ) {
        this.m_profileTypes = profiletypes;
    }


    public int doStartTag() throws JspException {
        try {

            if ( m_entityType.equals( "device") ) {

                if ( m_deviceAdvocateHome == null ) {
                    m_deviceAdvocateHome = ( DeviceAdvocateHome )
                        EJBHomeFactory.getInstance().getHomeInterface(  DeviceAdvocateHome.class,
                                                                        "DeviceAdvocate" );
                }

                DeviceAdvocate advocate = m_deviceAdvocateHome.create();
                advocate.generateProfiles( m_entityID, m_profileTypes, null );

            }
            else if  (  m_entityType.equals( "user") ) {

                if ( m_userAdvocateHome == null ) {
                    m_userAdvocateHome = ( UserAdvocateHome )
                        EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                        "UserAdvocate" );
                }

                UserAdvocate advocate = m_userAdvocateHome.create();
                advocate.generateProfiles( m_entityID, m_profileTypes, null);
            }
            else if  (  m_entityType.equals( "devicegroup") ||
                        m_entityType.equals( "usergroup") ) {

                HashMap messageValues = new HashMap();
                messageValues.put( "entitytype", m_entityType );
                messageValues.put( "entityid", m_entityID );
                messageValues.put( "profiletypes", m_profileTypes );
                SendJMSMessage( "queue/A", messageValues );
            }
            else {
                throw new JspException ( "entitytype specified " + m_entityType + " is invalid" );
            }
        }
        catch (Exception ex ) {
            ex.printStackTrace();
            throw new JspTagException( ex.toString() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_entityType = null;
        m_entityID = null;
        m_profileTypes = null;

        super.clearProperties();
    }
}