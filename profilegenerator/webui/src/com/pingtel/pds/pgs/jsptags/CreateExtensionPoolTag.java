/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/CreateExtensionPoolTag.java#4 $
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
import com.pingtel.pds.pgs.beans.CreateExtensionPoolBean;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.PageContext;

/**
 * tag handler for creating an extension pool
 * @author IB
 */
public class CreateExtensionPoolTag extends ExTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String mName = null;
    private String mOrganizationID = null;
    private ExtensionPoolAdvocate mExtensionPoolAdvocateEJBObject = null;

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public void setName ( String name ) {
        if ( name != null )
            mName = name;
    }

    public void setOrganizationid ( String organizationid ) {
        mOrganizationID = organizationid;
    }


    public int doStartTag() throws JspException {

        try {
            if ( mExtensionPoolAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionPoolAdvocateEJBObject = uaHome.create();
            }

            mExtensionPoolAdvocateEJBObject.createExtensionPool( mOrganizationID, mName );
        }
        catch(  Exception ex ) {
            CreateExtensionPoolBean createExtPoolBean = new CreateExtensionPoolBean();
            createExtPoolBean.setName(mName);
            createExtPoolBean.setErrorMessage(ex.getMessage());

            pageContext.setAttribute("CreateExtPoolBean", createExtPoolBean, PageContext.REQUEST_SCOPE);

            throw new RedirectServletException(ex.getMessage(), "../popup/add_extpool.jsp", null);
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mName = null;
        mOrganizationID = null;

        super.clearProperties();
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
