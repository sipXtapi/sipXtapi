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

import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper;
import com.pingtel.pds.pgs.phone.DeviceTypeHelper;
import com.pingtel.pds.common.EJBHomeFactory;

import javax.servlet.jsp.JspException;

import org.jdom.Element;

import java.util.HashMap;

/**
 * DeviceTabsTag it the tag handler that builds the 'tabs' for the Device
 * Details page.
 *
 * @author ibutcher
 * 
 */
public class DeviceTabsTag extends StyleTagSupport{

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String deviceid;
    private String refpropertyproupid;
    private String devicetypeid;
    private String mfgid;
    private String usertype;
    private String propname;
    private String vcuid;
    private String level;
    private String visiblefor;
    private String detailstype;

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Standard JavaBean implementation
     */
    public void setDeviceid ( String deviceid ) {
        this.deviceid = deviceid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setRefpropertygroupid ( String refpropertygroupid ) {
        this.refpropertyproupid = refpropertygroupid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setDevicetypeid ( String devicetypeid ) {
        this.devicetypeid = devicetypeid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setMfgid ( String mfgid ) {
        this.mfgid = mfgid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setUsertype ( String usertype ) {
        this.usertype = usertype;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setPropname ( String propname ) {
        this.propname = propname;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setVcuid ( String vcuid ) {
        this.vcuid = vcuid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setDetailstype ( String detailstype ) {
        this.detailstype = detailstype;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setLevel ( String level ) {
        this.level = level;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setVisiblefor ( String visiblefor ) {
        this.visiblefor = visiblefor;
    }


    /**
     * Standard TagHandler implementation.
     * @return always return SKIP_BODY.
     * @throws JspException for all errors.
     */
    public int doStartTag() throws JspException {
        try {
            TagHandlerHelperHome tagHanlderHelperHome = (TagHandlerHelperHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  TagHandlerHelperHome.class,
                                                                    "TagHandlerHelper" );

            TagHandlerHelper tagHandlerHelper = tagHanlderHelperHome.create();

            Element documentRootElement = tagHandlerHelper.getDeviceTabs(deviceid);

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put(   "refpropertygroupid",
                                        this.refpropertyproupid );
            stylesheetParameters.put(   "devicetypeid",
                                        this.devicetypeid );
            stylesheetParameters.put(   "mfgid",
                                        this.mfgid );
            stylesheetParameters.put(   "usertype",
                                        this.usertype );
            stylesheetParameters.put(   "propname",
                                        this.propname );
            stylesheetParameters.put(   "vcuid",
                                        this.vcuid );
            stylesheetParameters.put(   "detailstype",
                                        this.detailstype );
            stylesheetParameters.put(   "level",
                                        this.level );
            stylesheetParameters.put(   "visiblefor",
                                        this.visiblefor );
            stylesheetParameters.put(   "modelLabel",
                    DeviceTypeHelper.getLabel(this.devicetypeid, this.mfgid));

            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch( Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        deviceid = null;
        refpropertyproupid = null;
        devicetypeid = null;
        mfgid = null;
        usertype = null;
        propname = null;
        vcuid = null;
        level = null;
        visiblefor = null;
        detailstype = null;

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
