/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/X_SetUsersPasswordTag.java#4 $
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
import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;

import javax.management.MBeanServer;
import javax.management.MBeanServerFactory;
import javax.management.ObjectName;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class X_SetUsersPasswordTag extends ExTagSupport {

    private String m_userID = null;
    private String m_password = null;
    private String m_oldPassword = null;
    private String m_parameters = null;

    private UserHome userHome = null;
    private OrganizationHome orgHome = null;
    private UserAdvocate m_userAdvocate = null;
    private MD5Encoder m_md5 = new MD5Encoder();

    public void setUserid( String userid ){
        m_userID = userid;
    }

    public void setPassword ( String password ) {
        m_password = password;
    }

    public void setOldpassword ( String oldpassword ) {
        m_oldPassword = oldpassword;
    }

    public void setParameters ( String parameters ) {
        m_parameters = parameters;
        System.out.println( "set parameters to " + m_parameters ) ;
    }


    public int doStartTag() throws JspException {
        try {
            if ( userHome == null ) {
                userHome = ( UserHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                    "User" );

                orgHome = ( OrganizationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  OrganizationHome.class,
                                                                    "Organization" );

                UserAdvocateHome userAdvocateHome = (UserAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                    "UserAdvocate" );

                m_userAdvocate = userAdvocateHome.create();
            }


            User user = userHome.findByPrimaryKey( m_userID );

            Organization org =
                orgHome.findByPrimaryKey( user.getOrganizationID() );

            String newStyleOldPassword =
                m_userAdvocate.digestUsersPassword (    user.getDisplayID(),
                                                        org.getDNSDomain(),
                                                        m_oldPassword );

            String oldStyleOldPassword =
                calculateOldStylePasstoken( user.getDisplayID(),
                                            org.getDNSDomain(),
                                            m_oldPassword );

            if ( user.getPassword() == null || user.getPassword().length() != 32 ) {

                if ( user.getPassword() == null ) {
                    if ( m_password != null || m_password != "" )
                        throw new RedirectServletException ( "Old PIN does not match existing PIN",
                                                        "../sipxchange/pinmgmnt.jsp",
                                                        m_parameters );
                }
                else {
                    if ( !user.getPassword().equals( m_password ) )
                        throw new RedirectServletException ( "Old PIN does not match existing PIN",
                                                        "../sipxchange/pinmgmnt.jsp",
                                                        m_parameters );
                }

            }
            else {
                if ( !user.getPassword().equals ( newStyleOldPassword ) &&
                        !user.getPassword().equals ( oldStyleOldPassword )  ) {

                    throw new RedirectServletException ( "Old PIN does not match existing PIN",
                                                        "../sipxchange/pinmgmnt.jsp",
                                                        m_parameters );
                }

            }

            String displayID = user.getDisplayID();


            m_userAdvocate.setUsersPassword( m_userID, displayID, m_password );

            m_userAdvocate.generateProfiles(    m_userID,
                                                String.valueOf( PDSDefinitions.PROF_TYPE_USER ),
                                                null );

            ////////////////////////////////////////////////////////////////////////////
            //
            // This code was added to work around JBoss' credential caching 'feature'
            // whereby it caches passwords for users for half an hour.   This means that
            // if a user changes their password both the old an the new are active until
            // the period has elapsed.   I modified the code from a posting for version
            // 3.0 on a JBoss forum.   Many of the classes changed between v. 2 and 3
            // of JBoss so I got the equivalent classes from the JBoss 2.4.4 code itself.
            // I recommend caution changing this code.
            //
            ////////////////////////////////////////////////////////////////////////////
            MBeanServer server = (MBeanServer)
                MBeanServerFactory.findMBeanServer(null).iterator().next();

            ObjectName jaasMgr = new ObjectName("Security:name=JaasSecurityManager");
            Object[] params = {"other"};
            String[] signature = {"java.lang.String"};
            server.invoke(jaasMgr, "flushAuthenticationCache", params, signature);

            System.out.println("[Logoff Action] Auth Cache Flushed");
        }
        catch ( Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }


    private String calculateOldStylePasstoken(String realUserID, String realm, String password) {
        return m_md5.encode( realUserID + ":" + realm.trim() + ":" + password.trim() );
    }

    protected void clearProperties() {
        m_userID = null;
        m_password = null;
        m_oldPassword = null;
        m_parameters = null;

        super.clearProperties();
    }
}
