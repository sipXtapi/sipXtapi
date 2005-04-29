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
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;

import javax.servlet.jsp.JspException;

/**
 */
public class CreateDeviceTag extends ExTagSupport {

    private String m_shortName;
    private String m_orgID;
    private String m_coreSoftwareDetailsID;
    private String m_refConfigSetID;
    private String m_description;
    private String m_deviceGroupID;
    private String m_serialNumber;
    private String m_deviceTypeID;
    private String m_parameterString;

    private DeviceAdvocateHome m_deviceAdvocateHome = null;
    private DeviceAdvocate m_deviceAdvocate = null;


    public void setShortname( String shortName ) {
        m_shortName = shortName;
    }

    public void setOrgid( String orgid ) {
        m_orgID = orgid;
    }

    public void setCoresoftwaredetailsid( String coresoftwaredetailsid ) {
        m_coreSoftwareDetailsID = coresoftwaredetailsid;
    }

    public void setRefconfigsetid( String refconfigsetid ) {
        m_refConfigSetID = refconfigsetid;
    }

    public void setDescription( String description ){
        m_description = description;
    }

    public void setDevicegroupid( String devicegroupid ){
        m_deviceGroupID = devicegroupid;
    }

    public void setSerialnumber ( String serialnumber ) {
        m_serialNumber = serialnumber;
    }

    public void setDevicetypeid ( String devicetypeid ) {
        this.m_deviceTypeID = devicetypeid;
    }

    public void setParameterstring ( String parameterstring ) {
        m_parameterString = parameterstring;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_deviceAdvocateHome == null ) {
                m_deviceAdvocateHome = ( DeviceAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceAdvocateHome.class,
                                                                    "DeviceAdvocate" );

                m_deviceAdvocate = m_deviceAdvocateHome.create();
            }

            Device device =
                m_deviceAdvocate.createDevice(  m_shortName,
                                                getOrganizationID(),
                                                m_coreSoftwareDetailsID,
                                                m_refConfigSetID,
                                                m_description,
                                                m_deviceGroupID,
                                                null,
                                                m_deviceTypeID,
                                                m_serialNumber );


        }
        catch (Exception ex ) {
            throw new RedirectServletException ( ex.getMessage(), "../popup/form_add_device.jsp", m_parameterString );
        }

        return SKIP_BODY;
    }


    protected void clearProperties()
    {
        m_shortName = null;
        m_orgID = null;
        m_coreSoftwareDetailsID = null;
        m_refConfigSetID = null;
        m_description = null;
        m_deviceGroupID = null;
        m_serialNumber = null;
        m_deviceTypeID = null;
        m_parameterString = null;

        super.clearProperties();
    }
}
