/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/UserDetailsTag.java#4 $
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
import com.pingtel.pds.common.PropertyGroupLink;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelper;
import com.pingtel.pds.pgs.jsptags.ejb.TagHandlerHelperHome;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import javax.servlet.jsp.JspException;
import java.io.FileNotFoundException;
import java.util.HashMap;


public class UserDetailsTag extends StyleTagSupport {

    private String m_userID;
    private String m_refPropertyGroupID;
    private String m_deviceTypeID;
    private String m_mfgID;
    private String m_userType;
    private String m_propName;
    private String m_vcuid;
    private String m_level;
    private String m_visiblefor;
    private String m_detailsType;
    private Element m_mappingsDoc = null;
    private SAXBuilder m_sb = new SAXBuilder();

    private TagHandlerHelper m_tagHanderHelper = null;


    public void setUserid ( String userid ) {
        this.m_userID = userid;
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
            // OPtimization to look up all home interfaces here
            if ( m_tagHanderHelper == null ) {

                TagHandlerHelperHome thhm = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface(
                            TagHandlerHelperHome.class,
                            "TagHandlerHelper" );

                m_tagHanderHelper = thhm.create();
            }


            System.out.println( "Calling TagHandlerHelperBean with userid: " + m_userID );
            Element documentRootElement = m_tagHanderHelper.getUserDetails( m_userID );


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
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    private Element getMappingsDoc() throws JDOMException, FileNotFoundException {
        return PropertyGroupLink.getInstance().getMappings();
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_userID = null;

        super.clearProperties();
    }

}