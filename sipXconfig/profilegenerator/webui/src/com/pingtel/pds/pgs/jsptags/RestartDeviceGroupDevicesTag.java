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

import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Queue;
import javax.jms.QueueConnection;
import javax.jms.QueueConnectionFactory;
import javax.jms.QueueSender;
import javax.jms.QueueSession;
import javax.jms.Session;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

import java.util.HashMap;

import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;


public class RestartDeviceGroupDevicesTag extends JMSTagHandler {


    private String m_deviceGroupID;

    public void setDevicegroupid( String devicegroupid ) {
        m_deviceGroupID = devicegroupid;
    }


    public int doStartTag() throws JspException {

        HashMap messageValues = new HashMap();
        messageValues.put( "entitytype", "devicegroup" );
        messageValues.put( "entityid", m_deviceGroupID );
        SendJMSMessage( "queue/C", messageValues );

        return SKIP_BODY;
    }


    protected void clearProperties()
    {
        m_deviceGroupID = null;

        super.clearProperties();
    }
}
