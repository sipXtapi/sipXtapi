/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/DeleteExtensionRangeTag.java#4 $
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

/**
 * DeleteExtensionRangeTag is a tag hanlder to delete a range of extensions
 * from an existing extension pool
 *
 * @author IB
 */
public class DeleteExtensionRangeTag extends ExTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private String mMinExtension = null;
    private String mMaxExtension = null;
    private String mExtensionPoolID = null;
    private ExtensionPoolAdvocate mExtensionAdvocateEJBObject = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    public void setMinextension ( String minextension ) {
        mMinExtension = minextension;
    }

    public void setMaxextension ( String maxextension ) {
        mMaxExtension = maxextension;
    }

    public void setExtensionpoolid ( String extensionpoolid ) {
        mExtensionPoolID = extensionpoolid;
    }

    public int doStartTag() throws JspException {

        try {
            if ( mExtensionAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionAdvocateEJBObject = uaHome.create();
            }

            mExtensionAdvocateEJBObject.deleteExtensionRange( mExtensionPoolID, mMinExtension, mMaxExtension );
        }
        catch( Exception ex ) {
            throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        mMinExtension = null;
        mMaxExtension = null;
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