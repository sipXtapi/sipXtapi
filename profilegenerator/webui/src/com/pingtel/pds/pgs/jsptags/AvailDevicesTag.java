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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class AvailDevicesTag extends ExTagSupport {

   private TagHandlerHelper m_tagHandlerHelperEJBObject;


    public int doStartTag() throws JspException  {
        try {
            if ( m_tagHandlerHelperEJBObject == null ) {
                TagHandlerHelperHome tagHandlerHome = (TagHandlerHelperHome)
                        EJBHomeFactory.getInstance().getHomeInterface( TagHandlerHelperHome.class,
                                                                        "TagHandlerHelper" );

                m_tagHandlerHelperEJBObject = tagHandlerHome.create();
            }

            pageContext.getOut().print( m_tagHandlerHelperEJBObject.getAvailableDevicesList() );
        }
        catch (Exception ioe ) {
            throw new JspTagException( ioe.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        super.clearProperties();
    }
}