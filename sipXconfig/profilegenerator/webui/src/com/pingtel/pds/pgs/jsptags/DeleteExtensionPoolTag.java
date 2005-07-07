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


import javax.servlet.jsp.JspException;
import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;


public class DeleteExtensionPoolTag extends ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private String mID = null;
    private ExtensionPoolAdvocate mExtensionAdvocateEJBObject = null;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    public void setId ( String id ) {
        if ( id != null )
            mID = id;
    }

    public int doStartTag() throws JspException {

        try {
            if ( mExtensionAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionAdvocateEJBObject = uaHome.create();
            }

            mExtensionAdvocateEJBObject.deleteExtensionPool( mID );
        }
        catch(  Exception ex ) {
            throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mID = null;

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