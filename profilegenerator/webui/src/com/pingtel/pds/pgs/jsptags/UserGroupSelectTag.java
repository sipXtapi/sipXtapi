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
import com.pingtel.pds.pgs.jsptags.util.CaseInsensitiveComparator;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome;

import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.UserGroupHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.*;

import org.jdom.Element;


public class UserGroupSelectTag extends StyleTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String usergroupid;

    // EJB Home references
    private TagHandlerHelper mTagHandlerHelperEJBObject = null;



//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Standard JavaBean implementation
     */
    public String getUsergroupid() {
        return usergroupid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setUsergroupid(String usergroupid) {
        this.usergroupid = usergroupid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( mTagHandlerHelperEJBObject == null ) {
                TagHandlerHelperHome home = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface( TagHandlerHelperHome.class,"TagHandlerHelper");

                mTagHandlerHelperEJBObject = home.create();
            }

            Element documentRootElement = null;

            if (usergroupid == null) {
                documentRootElement = mTagHandlerHelperEJBObject.getUserGroupTree();
            }
            else {
                documentRootElement = mTagHandlerHelperEJBObject.getUserGroupTree(new Integer(usergroupid));
            }

            outputTextToBrowser (documentRootElement);
        }
        catch (Exception e) {
            throw new JspTagException(e.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        usergroupid = null;
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






