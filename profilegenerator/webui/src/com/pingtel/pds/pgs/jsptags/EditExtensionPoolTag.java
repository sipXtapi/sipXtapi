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

import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.common.EJBHomeFactory;

import javax.servlet.jsp.JspException;

/**
 * EditExtensionPoolTag is a tag hanlder to edit and existing ExtensionPool.
 *
 * @author IB
 */
public class EditExtensionPoolTag extends ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private String mName = null;
    private String mExtensionPoolId = null;
    private ExtensionPoolAdvocate mExtensionPoolAdvocateEJBObject = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    public void setName ( String name ) {
        mName = name;
    }
    public void setExtensionpoolid ( String extensionpoolid ) {
        mExtensionPoolId = extensionpoolid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( mExtensionPoolAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionPoolAdvocateEJBObject = uaHome.create();
            }
            mExtensionPoolAdvocateEJBObject.editExtensionPool( mExtensionPoolId, mName);
        }
        catch ( Exception e) {
           throw new JspException( e.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mName = null;
        mExtensionPoolId = null;

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


