/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/UserGroupDetailsTag.java#4 $
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
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.HashMap;

/**
 * UserGroupDetailsTag is a tag handler which produces the 'tabbed' user group
 * details page.   It delegates most of its processing to the
 * TagHandlerHelper EJ Bean.
 *
 * @author IB
 */
public class UserGroupDetailsTag extends StyleTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private Integer mUserGroupID;
    private String mRefPropertyGroupID;
    private String mDeviceTypeID;
    private String mManufacturerID;
    private String mUserType;
    private String mPropertyName;
    private String mVCUID;
    private String mLevel;
    private String mVisiblefor;
    private String mDetailsType;

    // EJ Bean references
    private TagHandlerHelper mTagHanderHelperEJBObject = null;
    private UserGroupHome mUserGroupHome = null;

    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    // JavaBean accessor/mutators
    public void setUsergroupid ( String usergroupid ) {
        mUserGroupID = new Integer ( usergroupid );
    }

    public void setRefpropertygroupid ( String refpropertygroupid ) {
        mRefPropertyGroupID = refpropertygroupid;
    }

    public void setDevicetypeid ( String devicetypeid ) {
        mDeviceTypeID = devicetypeid;
    }

    public void setMfgid ( String mfgid ) {
        mManufacturerID = mfgid;
    }

    public void setUsertype ( String usertype ) {
        mUserType = usertype;
    }

    public void setPropname ( String propname ) {
        mPropertyName = propname;
    }

    public void setVcuid ( String vcuid ) {
        mVCUID = vcuid;
    }

    public void setDetailstype ( String detailstype ) {
        mDetailsType = detailstype;
    }

    public void setLevel ( String level ) {
        mLevel = level;
    }

    public void setVisiblefor ( String visiblefor ) {
        mVisiblefor = visiblefor;
    }


    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            if ( mTagHanderHelperEJBObject == null ) {
                TagHandlerHelperHome thhm = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                            TagHandlerHelperHome.class,
                            "TagHandlerHelper" );

                mTagHanderHelperEJBObject = thhm.create();

                mUserGroupHome = (UserGroupHome)
                    EJBHomeFactory.getInstance().getHomeInterface(
                            UserGroupHome.class, "UserGroup" );
            }

            UserGroup userGroup = mUserGroupHome.findByPrimaryKey( mUserGroupID );

            Element documentRootElement =
                    mTagHanderHelperEJBObject.getUserGroupDetails( userGroup );

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put( "refpropertygroupid", mRefPropertyGroupID );
            stylesheetParameters.put( "devicetypeid", mDeviceTypeID );
            stylesheetParameters.put( "mfgid", mManufacturerID );
            stylesheetParameters.put( "usertype", mUserType );
            stylesheetParameters.put( "propname", mPropertyName );
            stylesheetParameters.put( "vcuid", mVCUID );
            stylesheetParameters.put( "detailstype", mDetailsType );
            stylesheetParameters.put( "level", mLevel );
            stylesheetParameters.put( "visiblefor", mVisiblefor );

            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootElement, stylesheetParameters );
        }
        catch(  Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        mUserGroupID = null;

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