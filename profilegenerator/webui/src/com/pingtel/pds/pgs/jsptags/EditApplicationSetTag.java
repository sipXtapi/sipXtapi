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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocate;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

/**
 * EditApplicationSetTag is the tag handler to allow users to modify the
 * attributes of an existing ApplicationSet.   Currently you can only
 * change the name of the ApplicationSet.
 *
 * @author IB
 */
public class EditApplicationSetTag extends ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    private String mApplicationSetID;
    private String mName;

    private ApplicationGroupAdvocate mApplicationGroupAdvocateEJBObject = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    public void setApplicationsetid ( String applicationsetid ) {
        mApplicationSetID = applicationsetid;
    }

    public void setName ( String name ) {
        mName = name;
    }


    public int doStartTag() throws JspException {
        try {
            if (mApplicationGroupAdvocateEJBObject == null) {
                ApplicationGroupAdvocateHome aaHome = ( ApplicationGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(
                            ApplicationGroupAdvocateHome.class,
                           "ApplicationGroupAdvocate" );

                mApplicationGroupAdvocateEJBObject = aaHome.create();
            }

            mApplicationGroupAdvocateEJBObject.editApplicationGroup(mApplicationSetID, mName);
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        mApplicationSetID = null;
        mName  = null;

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
