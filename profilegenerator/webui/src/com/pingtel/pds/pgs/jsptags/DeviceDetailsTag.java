/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/DeviceDetailsTag.java#4 $
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



public class DeviceDetailsTag extends StyleTagSupport {

    private String m_deviceID;
    private String m_refPropertyGroupID;
    private String m_deviceTypeID;
    private String m_mfgID;
    private String m_userType;
    private String m_propName;
    private String m_vcuid;
    private String m_level;
    private String m_visiblefor;
    private String m_detailsType;

    public void setDeviceid ( String deviceid ) {
        this.m_deviceID = deviceid;
    }

    public void setRefpropertygroupid ( String refpropertygroupid ) {
        this.m_refPropertyGroupID = refpropertygroupid;
    }

    public void setDevicetypeid ( String devicetypeid ) {
        this.m_deviceTypeID = devicetypeid;
    }

    public void setMfgid ( String mfgid ) {
        this.m_mfgID = mfgid;
    }

    public void setUsertype ( String usertype ) {
        this.m_userType = usertype;
    }

    public void setPropname ( String propname ) {
        this.m_propName = propname;
    }

    public void setVcuid ( String vcuid ) {
        this.m_vcuid = vcuid;
    }

    public void setDetailstype ( String detailstype ) {
        this.m_detailsType = detailstype;
    }

    public void setLevel ( String level ) {
        this.m_level = level;
    }

    public void setVisiblefor ( String visiblefor ) {
        this.m_visiblefor = visiblefor;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            TagHandlerHelperHome tagHanlderHelperHome = (TagHandlerHelperHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  TagHandlerHelperHome.class,
                                                                    "TagHandlerHelper" );

            TagHandlerHelper tagHandlerHelper = tagHanlderHelperHome.create();

            Element documentRootElement = tagHandlerHelper.getDeviceDetails( m_deviceID );

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put(   "refpropertygroupid",
                                        this.m_refPropertyGroupID );
            stylesheetParameters.put(   "devicetypeid",
                                        this.m_deviceTypeID );
            stylesheetParameters.put(   "mfgid",
                                        this.m_mfgID );
            stylesheetParameters.put(   "usertype",
                                        this.m_userType );
            stylesheetParameters.put(   "propname",
                                        this.m_propName );
            stylesheetParameters.put(   "vcuid",
                                        this.m_vcuid );
            stylesheetParameters.put(   "detailstype",
                                        this.m_detailsType );
            stylesheetParameters.put(   "level",
                                        this.m_level );
            stylesheetParameters.put(   "visiblefor",
                                        this.m_visiblefor );

            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch( Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_deviceID = null;

        super.clearProperties();
    }
}