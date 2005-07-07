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
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.HashMap;


/**
 * <p>Title: ListDevicesTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */


public class ListUsersTag extends StyleTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private String groupid = null;

    private TagHandlerHelper mTagHandlerHelperEJBObject = null;

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////


    public void setGroupid ( String groupid ) {
        this.groupid = groupid;
    }


    public int doStartTag() throws JspException {
        try {
            java.util.Date d1 = new java.util.Date();
            if ( mTagHandlerHelperEJBObject == null ) {

                TagHandlerHelperHome helperHome = ( TagHandlerHelperHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  TagHandlerHelperHome.class,
                                                                    "TagHandlerHelper" );

                mTagHandlerHelperEJBObject = helperHome.create();
            }

            Element root = null;

            if (groupid != null && groupid.equalsIgnoreCase("all")) {
                root = mTagHandlerHelperEJBObject.listAllUsers();
            } else {
                if (groupid != null){
                    root = mTagHandlerHelperEJBObject.listUsers(new Integer(groupid));
                } else {
                    root = mTagHandlerHelperEJBObject.listUsers(null);
                }

            }

            HashMap stylesheetParameters = new HashMap();
            String idTest = "";
            if( this.groupid != null ) {
                idTest = this.groupid.toString();
            }
            else {
                idTest = "empty";
            }

            stylesheetParameters.put( "idtest", idTest );

            outputTextToBrowser ( root, stylesheetParameters );
            java.util.Date d2 = new java.util.Date();
            System.out.println( "IT TOOK " + ( d2.getTime() - d1.getTime() ) + "ms" );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }



    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        groupid = null;
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