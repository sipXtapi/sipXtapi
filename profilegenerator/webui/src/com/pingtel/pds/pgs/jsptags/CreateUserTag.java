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
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;

import javax.servlet.jsp.JspException;


/**
 * <p>Title: </p>
 * <p>Description: </p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: </p>
 * @author unascribed
 * @version 1.0
 */

public class CreateUserTag extends ExTagSupport {

    private String m_firstName = null;
    private String m_lastName = null;
    private String m_password = null;
    private String m_displayID = null;
    private String m_refConigSetID = null;
    private String m_userGroupID = null;
    private String m_extensionNumber = null;
    private String m_parameterString = null;


    private UserAdvocateHome uaHome = null;
    private UserAdvocate uAdvocate = null;

    public void setFirstname( String firstname ){
        m_firstName = firstname;
    }

    public void setLastname( String lastname ){
        m_lastName = lastname;
    }

    public void setPassword( String password ){
        m_password = password;
    }

    public void setDisplayid( String displayid ){
        m_displayID = displayid;
    }

    public void setRefconfigsetid( String refconfigsetid ){
        m_refConigSetID = refconfigsetid;
    }

    public void setUsergroupid( String usergroupid ){
        m_userGroupID = usergroupid;
    }

    public void setExtensionnumber ( String extensionnumber ) {
        m_extensionNumber = extensionnumber;
    }

    public void setParameterstring ( String parameterstring ) {
        m_parameterString = parameterstring;
    }

    /**
     *
     * @return
     * @exception JspException
     */
    public int doStartTag() throws JspException {
        try {
            if ( uaHome == null ) {
                uaHome = ( UserAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                    "UserAdvocate" );

                uAdvocate = uaHome.create();
            }

            User user = uAdvocate.createUser(   getOrganizationID(),
                                                m_firstName,
                                                m_lastName,
                                                m_password,
                                                m_displayID,
                                                m_refConigSetID,
                                                m_userGroupID,
                                                m_extensionNumber );

        }
        catch (Exception ex ) {
            throw new RedirectServletException ( ex.getMessage(), "../popup/form_add_user.jsp", m_parameterString );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_firstName = null;
        m_lastName = null;
        m_password = null;
        m_displayID = null;
        m_refConigSetID = null;
        m_userGroupID = null;
        m_extensionNumber = null;

        super.clearProperties();
    }
}
