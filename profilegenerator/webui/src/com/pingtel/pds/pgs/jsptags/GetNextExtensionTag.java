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
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;


public class GetNextExtensionTag extends com.pingtel.pds.pgs.jsptags.util.ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private String mExtensionPoolID = null;
    private ExtensionPoolAdvocate mExtensionAdvocateEJBObject = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    public void setExtensionpoolid ( String extensionpoolid ) {
        if ( extensionpoolid != null )
            mExtensionPoolID = extensionpoolid;
    }


    public int doStartTag() throws javax.servlet.jsp.JspException {


        try {
            if ( mExtensionAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionAdvocateEJBObject = uaHome.create();
            }

            String nextValue =
                mExtensionAdvocateEJBObject.getNextExtension( mExtensionPoolID );

            pageContext.getOut().print( nextValue );
        }
        catch( Exception ex ) {
            throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mExtensionPoolID = null;

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
