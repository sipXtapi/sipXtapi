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
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocate;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class DeleteDeviceGroupTag extends ExTagSupport
{
    private String m_deviceGroupID = null;
    private String m_option = null;
    private String m_newDeviceGroupID = null;
    private DeviceGroupAdvocateHome dgaHome = null;
    private DeviceGroupAdvocate dgAdvocate = null;

    public void setDevicegroupid( String devicegroupid ) {
        m_deviceGroupID = devicegroupid;
    }

    public void setOption( String option ) {
        m_option = option;
    }

    public void setNewdevicegroupid( String newdevicegroupid ) {
        m_newDeviceGroupID = newdevicegroupid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( dgaHome == null ) {
                dgaHome = ( DeviceGroupAdvocateHome )
                                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupAdvocateHome.class,
                                                                                    "DeviceGroupAdvocate" );

                dgAdvocate = dgaHome.create();
            }

            dgAdvocate.deleteDeviceGroup(   m_deviceGroupID,
                                            m_option,
                                            m_newDeviceGroupID );
        }
        catch(Exception ex ) {
            throw new JspTagException( ex.getMessage());
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_deviceGroupID = null;
        m_option = null;
        m_newDeviceGroupID = null;

        super.clearProperties();
    }
}