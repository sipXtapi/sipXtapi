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
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocate;
import com.pingtel.pds.pgs.phone.DeviceGroupAdvocateHome;

import javax.servlet.jsp.JspException;


public class CreateDeviceGroupTag extends ExTagSupport {

    private String m_refConfigSetID = null;
    private String m_groupName = null;
    private String m_parentID = null;
    private String m_parameterString = null;

    private DeviceGroupAdvocateHome dgHome = null;
    private DeviceGroupAdvocate dgAdvocate = null;


    public void setRefconfigsetid ( String refconfigsetid ) {
        this.m_refConfigSetID = refconfigsetid;
    }

    public void setGroupname( String groupname ){
        m_groupName = groupname;
    }

    public void setParentgroupid ( String parentgroupid ) {
        m_parentID = parentgroupid;
    }

    public void setParameterstring ( String parameterstring ) {
        m_parameterString = parameterstring;
    }



    public int doStartTag() throws JspException {
        try {
            if ( dgHome == null ) {
                dgHome = ( DeviceGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  DeviceGroupAdvocateHome.class,
                                                                    "DeviceGroupAdvocate" );

                dgAdvocate = dgHome.create();
            }

            dgAdvocate.createDeviceGroup(   getOrganizationID(),
                                            m_groupName,
                                            m_refConfigSetID,
                                            m_parentID,
                                            null );

        }
        catch (Exception ex ) {
            throw new RedirectServletException ( ex.getMessage(),
                                                "../popup/form_add_devicegroup.jsp",
                                                m_parameterString );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_refConfigSetID = null;
        m_groupName = null;
        m_parentID = null;
        m_parameterString = null;

        super.clearProperties();
    }
}
