/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/RestartUserGroupDevicesTag.java#4 $
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

import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;


public class RestartUserGroupDevicesTag extends ExTagSupport {

    private String m_userGroupID;

    private static Queue m_queue = null;
    private static QueueSender m_queueSender = null;


    public void setUsergroupid( String usergroupid ) {
        m_userGroupID = usergroupid;
    }


    public int doStartTag() throws JspException {
        try {
            QueueSession queueSession = null;

            if ( m_queue == null ) {
                Context context = new InitialContext();

                // Get the connection factory
                QueueConnectionFactory queueFactory =
                    (QueueConnectionFactory)context.lookup("QueueConnectionFactory");


                // Create the connection
                QueueConnection queueConnection = queueFactory.createQueueConnection();

                // Create the session
                queueSession =
                        queueConnection.createQueueSession( // No transaction
                                                            false,
                                                            // Auto ack
                                                            Session.AUTO_ACKNOWLEDGE);

                m_queue = (Queue)context.lookup("queue/C");

                // Create a sender
                m_queueSender = queueSession.createSender(m_queue);
            }

            // Create a message
            MapMessage message = queueSession.createMapMessage();
            message.setString ( "entitytype", "usergroup" );
            message.setString ( "entityid", m_userGroupID );

            // Send the message
            m_queueSender.send(m_queue, message);
        }
        catch (NamingException e) {
            throw new JspTagException( e.getMessage());
        }
        catch (JMSException e) {
            throw new JspTagException( e.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties()
    {
        m_userGroupID = null;

        super.clearProperties();
    }
}