/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/CopyDeviceGroupTag.java#4 $
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
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocate;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class CopyDeviceGroupTag extends ExTagSupport
{
    private String m_sourceDeviceGroupID = null;

    private DeviceGroupAdvocateHome dgaHome = null;
    private DeviceGroupAdvocate dgAdvocate = null;

    public void setSourcedevicegroupid( String sourcedevicegroupid ) {
        m_sourceDeviceGroupID = sourcedevicegroupid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( dgaHome == null ) {
                dgaHome = ( DeviceGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupAdvocateHome.class,
                                                                    "DeviceGroupAdvocate" );

                dgAdvocate = dgaHome.create();
            }

            dgAdvocate.copyDeviceGroup( this.m_sourceDeviceGroupID );

        }
        catch(Exception ex ) {
            throw new JspTagException( ex.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_sourceDeviceGroupID = null;

        super.clearProperties();
    }
}