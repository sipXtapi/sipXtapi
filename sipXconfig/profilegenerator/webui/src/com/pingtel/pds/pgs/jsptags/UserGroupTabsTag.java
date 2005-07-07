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
 * UserGroupTabsTag builds the 'tabs' or buttons for the UserGroup
 * Details page.
 *
 * @author ibutcher
 * 
 */
public class UserGroupTabsTag extends StyleTagSupport {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // JavaBean attributes
    private String usergroupid;
    private String refpropertygroupid;
    private String devicetypeid;
    private String manufacturerid;
    private String usertype;
    private String propertyname;
    private String vcuid;
    private String level;
    private String visiblefor;
    private String detailstype;

    // EJ Bean references
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
    public void setUsergroupid ( String usergroupid ) {
        this.usergroupid = usergroupid ;
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
        manufacturerid = mfgid;
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
        propertyname = propname;
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
            if ( mTagHanderHelperEJBObject == null ) {
                TagHandlerHelperHome thhm = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                            TagHandlerHelperHome.class,
                            "TagHandlerHelper" );

                mTagHanderHelperEJBObject = thhm.create();
            }


            Element documentRootElement =
                    mTagHanderHelperEJBObject.getUserGroupTabs(usergroupid);

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put( "refpropertygroupid", refpropertygroupid );
            stylesheetParameters.put( "devicetypeid", devicetypeid );
            stylesheetParameters.put( "mfgid", manufacturerid );
            stylesheetParameters.put( "usertype", usertype );
            stylesheetParameters.put( "propname", propertyname );
            stylesheetParameters.put( "vcuid", vcuid );
            stylesheetParameters.put( "detailstype", detailstype );
            stylesheetParameters.put( "level", level );
            stylesheetParameters.put( "visiblefor", visiblefor );

            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch(  Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
        usergroupid = null;
        refpropertygroupid = null;
        devicetypeid = null;
        manufacturerid = null;
        usertype = null;
        propertyname = null;
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
