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
import com.pingtel.pds.pgs.beans.CreateApplicationSetBean;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocate;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.PageContext;


/**
 * CreateApplicationGroupTag is a tag handler to create a new
 * ApplicationSet.
 *
 * @author ibutcher
 *
 */
public class CreateApplicationGroupTag extends ExTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String mName;

    // EJB Remote references
    private ApplicationGroupAdvocate applicationGroupAdvocateEJBObject = null;

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Standard JavaBean implementation
     */
    public void setName ( String name ) {
        mName = name;
    }


    public int doStartTag() throws JspException {
        try {
            if (applicationGroupAdvocateEJBObject == null) {
                ApplicationGroupAdvocateHome aaHome = ( ApplicationGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupAdvocateHome.class,
                                                                    "ApplicationGroupAdvocate" );

                applicationGroupAdvocateEJBObject = aaHome.create();
            }

            applicationGroupAdvocateEJBObject.createApplicationGroup( mName, getOrganizationID() );

        }
        catch (Exception ex ) {
            CreateApplicationSetBean createApplicationSetBean = new CreateApplicationSetBean();
            createApplicationSetBean.setName(mName);
            createApplicationSetBean.setErrorMessage(ex.getMessage());

            pageContext.setAttribute("CreateApplicationSetBean", createApplicationSetBean, PageContext.REQUEST_SCOPE);

            throw new RedirectServletException(ex.getMessage(), "../popup/form_add_appset.jsp", null);
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
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
