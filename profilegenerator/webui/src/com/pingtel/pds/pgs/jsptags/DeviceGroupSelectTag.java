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
import javax.servlet.jsp.JspTagException;

/**
 * Tag handler used to build a hierachical list of DeviceGroups.
 * It may take a devicegroupid attribute which if supplied is
 * used to filter the results so that only valid parent groups
 * are shown.
 */
public class DeviceGroupSelectTag extends StyleTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String devicegroupid;

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
    public String getDevicegroupid() {
        return devicegroupid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setDevicegroupid(String devicegroupid) {
        this.devicegroupid = devicegroupid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( mTagHandlerHelperEJBObject == null ) {
                TagHandlerHelperHome home = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface( TagHandlerHelperHome.class,"TagHandlerHelper");

                mTagHandlerHelperEJBObject = home.create();
            }

            Element documentRootElement = null;

            if (devicegroupid == null) {
                documentRootElement = mTagHandlerHelperEJBObject.getDeviceGroupTree();
            }
            else {
                documentRootElement = mTagHandlerHelperEJBObject.getDeviceGroupTree(new Integer(devicegroupid));
            }

            outputTextToBrowser (documentRootElement);
        }
        catch (Exception e) {
            throw new JspTagException(e.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        devicegroupid = null;
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