/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/UserTabsTag.java#4 $
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
 * UserTabsTag is the tag handler that builds the 'tab's or buttons
 * for the User Details page.
 *
 * @author ibutcher
 * 
 */
public class UserTabsTag extends StyleTagSupport{

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String userid;
    private String refpropertygroupid;
    private String devicetypeid;
    private String mfgid;
    private String usertype;
    private String propname;
    private String vcuid;
    private String level;
    private String visiblefor;
    private String detailstype;

    // EJB remote reference
    private TagHandlerHelper mTagHanderHelperEJBObject = null;


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     * Standard JavaBean implementation
     */
    public void setUserid ( String userid ) {
        this.userid = userid;
    }

    /**
     * Standard JavaBean implementation
     */
    public void setRefpropertygroupid ( String refpropertygroupid ) {
        this.refpropertygroupid = refpropertygroupid;
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
            if (mTagHanderHelperEJBObject == null) {

                TagHandlerHelperHome thhm = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                            TagHandlerHelperHome.class,
                            "TagHandlerHelper" );

                mTagHanderHelperEJBObject = thhm.create();
            }

            Element documentRootElement = mTagHanderHelperEJBObject.getUserTabs(userid);

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put("refpropertygroupid", this.refpropertygroupid);
            stylesheetParameters.put("devicetypeid", this.devicetypeid);
            stylesheetParameters.put("mfgid", this.mfgid);
            stylesheetParameters.put("usertype", this.usertype);
            stylesheetParameters.put("propname", this.propname);
            stylesheetParameters.put("vcuid", this.vcuid);
            stylesheetParameters.put("detailstype", this.detailstype);
            stylesheetParameters.put("level", this.level);
            stylesheetParameters.put("visiblefor", this.visiblefor);

            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        userid = null;
        refpropertygroupid = null;
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
