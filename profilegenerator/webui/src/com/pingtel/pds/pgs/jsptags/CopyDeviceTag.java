/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/CopyDeviceTag.java#4 $
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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class CopyDeviceTag extends ExTagSupport {

    private String m_sourceDeviceID;

    private DeviceAdvocateHome m_deviceAdvocateHome = null;
    private DeviceAdvocate m_deviceAdvocate = null;


    public void setSourcedeviceid( String sourcedeviceid ) {
        m_sourceDeviceID = sourcedeviceid;
    }



    public int doStartTag() throws JspException {
        try {
            if ( m_deviceAdvocateHome == null ) {
                m_deviceAdvocateHome = ( DeviceAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceAdvocateHome.class,
                                                                    "DeviceAdvocate" );

                m_deviceAdvocate = m_deviceAdvocateHome.create();
            }

            m_deviceAdvocate.copyDevice( m_sourceDeviceID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_sourceDeviceID = null;

        super.clearProperties();
    }
}