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

import java.io.IOException;
import java.util.HashMap;

import javax.ejb.CreateException;
import javax.ejb.FinderException;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;
import javax.xml.transform.TransformerException;

import org.jdom.Element;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupHome;

/**
 * UserGroupDetailsTag is a tag handler which produces the 'tabbed' user group details page. It
 * delegates most of its processing to the TagHandlerHelper EJ Bean.
 * 
 * @author IB
 */
public class UserGroupDetailsTag extends StyleTagSupport {

    // ////////////////////////////////////////////////////////////////////////
    // Constants
    // //

    // ////////////////////////////////////////////////////////////////////////
    // Attributes
    // //

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

    // ////////////////////////////////////////////////////////////////////////
    // Construction
    // //

    // ////////////////////////////////////////////////////////////////////////
    // Public Methods
    // //

    // JavaBean accessor/mutators
    public void setUsergroupid(String usergroupid) {
        mUserGroupID = new Integer(usergroupid);
    }

    public void setRefpropertygroupid(String refpropertygroupid) {
        mRefPropertyGroupID = refpropertygroupid;
    }

    public void setDevicetypeid(String devicetypeid) {
        mDeviceTypeID = devicetypeid;
    }

    public void setMfgid(String mfgid) {
        mManufacturerID = mfgid;
    }

    public void setUsertype(String usertype) {
        mUserType = usertype;
    }

    public void setPropname(String propname) {
        mPropertyName = propname;
    }

    public void setVcuid(String vcuid) {
        mVCUID = vcuid;
    }

    public void setDetailstype(String detailstype) {
        mDetailsType = detailstype;
    }

    public void setLevel(String level) {
        mLevel = level;
    }

    public void setVisiblefor(String visiblefor) {
        mVisiblefor = visiblefor;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            TagHandlerHelperHome thhm = (TagHandlerHelperHome) EJBHomeFactory.getInstance()
                    .getHomeInterface(TagHandlerHelperHome.class, "TagHandlerHelper");

            TagHandlerHelper tagHanderHelperEJBObject = thhm.create();

            UserGroupHome userGroupHome = (UserGroupHome) EJBHomeFactory.getInstance()
                    .getHomeInterface(UserGroupHome.class, "UserGroup");

            UserGroup userGroup = userGroupHome.findByPrimaryKey(mUserGroupID);

            Element documentRootElement = tagHanderHelperEJBObject.getUserGroupDetails(userGroup);

            HashMap stylesheetParameters = new HashMap();
            stylesheetParameters.put("refpropertygroupid", mRefPropertyGroupID);
            stylesheetParameters.put("devicetypeid", mDeviceTypeID);
            stylesheetParameters.put("mfgid", mManufacturerID);
            stylesheetParameters.put("usertype", mUserType);
            stylesheetParameters.put("propname", mPropertyName);
            stylesheetParameters.put("vcuid", mVCUID);
            stylesheetParameters.put("detailstype", mDetailsType);
            stylesheetParameters.put("level", mLevel);
            stylesheetParameters.put("visiblefor", mVisiblefor);

            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser(documentRootElement, stylesheetParameters);
            return SKIP_BODY;
        } catch (NamingException e) {
            throw new JspException(e);
        } catch (CreateException e) {
            throw new JspException(e);
        } catch (FinderException e) {
            throw new JspException(e);
        } catch (TransformerException e) {
            throw new JspException(e);
        } catch (IOException e) {
            throw new JspException(e);
        } catch (PDSException e) {
            throw new JspException(e);
        }
    }

    /**
     * Called by super class method (not by superClass's clearProperties method).
     */
    protected void clearProperties() {
        mUserGroupID = null;

        super.clearProperties();
    }

    // ////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    // //

    // ////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    // //

    // ////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    // //

}
