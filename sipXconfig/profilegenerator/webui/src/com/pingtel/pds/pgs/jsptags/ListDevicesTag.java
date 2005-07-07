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


public class ListDevicesTag extends StyleTagSupport {


    private Integer m_groupID = null;
    private TagHandlerHelper m_tagHandlerHelper = null;


    public void setGroupid ( String groupid ) {
        if ( groupid != null )
            m_groupID = Integer.valueOf( groupid );
    }


    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            if ( m_tagHandlerHelper == null ) {
                TagHandlerHelperHome helperHome = ( TagHandlerHelperHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  TagHandlerHelperHome.class,
                                                                    "TagHandlerHelper" );

                m_tagHandlerHelper = helperHome.create();
            }

            Element root = m_tagHandlerHelper.listDevices( m_groupID );

            HashMap stylesheetParameters = new HashMap();
            String idTest = "";
            if( this.m_groupID != null ) {
                idTest = this.m_groupID.toString();
            }else {
                idTest = "empty";
            }
            stylesheetParameters.put(   "idtest",
                                        idTest );
            outputTextToBrowser ( root, stylesheetParameters );
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
        m_groupID = null;

        super.clearProperties();
    }
}
