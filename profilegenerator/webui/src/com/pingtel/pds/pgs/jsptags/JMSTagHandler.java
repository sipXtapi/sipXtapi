/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/JMSTagHandler.java#4 $
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

import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;

import javax.jms.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;
import java.util.Iterator;
import java.util.Map;


public class JMSTagHandler extends ExTagSupport {
    protected static final String DATASET_TYPE_MSG_KEY = "datasettype";
    protected static final String DATASET_QUEUE_NAME = "queue/B";

    protected void SendJMSMessage ( String queueName, Map mapMessageValues )
            throws JspException {

        QueueConnection queueConnection = null;

        try {
            Context context = new InitialContext();

            // Get the connection factory
            QueueConnectionFactory queueFactory =
                (QueueConnectionFactory)context.lookup("QueueConnectionFactory");


            // Create the connection
            queueConnection = queueFactory.createQueueConnection();

            // Create the session
            QueueSession queueSession =
                    queueConnection.createQueueSession( // No transaction
                                                        false,
                                                        // Auto ack
                                                        Session.AUTO_ACKNOWLEDGE);

            Queue queue = (Queue)context.lookup( queueName );

            // Create a sender
            QueueSender queueSender = queueSession.createSender(queue);

            MapMessage message = queueSession.createMapMessage();

            for ( Iterator i = mapMessageValues.entrySet().iterator(); i.hasNext(); ) {
                Map.Entry entry = (Map.Entry) i.next();
                String name = (String) entry.getKey();
                String value = (String) entry.getValue();

                message.setString( name, value );
            }

            // Send the message
            queueSender.send( queue, message);
        }
        catch ( NamingException e ) {
            e.printStackTrace();
            throw new JspTagException( e.getMessage() );
        }
        catch ( JMSException e ) {
            e.printStackTrace();
            throw new JspTagException( e.getMessage() );
        }
        finally {
            try {
                if ( queueConnection != null )
                    queueConnection.close();
            }
            catch ( JMSException e ) {
                throw new JspException( e.getMessage() );
            }
        }

    }
}
