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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class DeleteDeviceTag extends ExTagSupport {

    private String m_deviceID;
    private DeviceAdvocateHome devaHome = null;
    private DeviceAdvocate devAdvocate = null;

    public void setDeviceid( String deviceid ) {
        m_deviceID = deviceid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( devaHome == null ) {
                devaHome = ( DeviceAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceAdvocateHome.class,
                                                                    "DeviceAdvocate" );

                devAdvocate = devaHome.create();
            }

            devAdvocate.deleteDevice( m_deviceID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties(){
        m_deviceID = null;

        super.clearProperties();
    }
}
