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

package com.pingtel.pds.pgs.soap;

import java.util.Iterator;
import java.util.Map;

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

/**
 * JMSHelper holds common methods and constants used by the Services to
 * contact and execute MDB.
 */
class JMSHelper {

//////////////////////////////////////////////////////////////////////////
// Constants
////

    protected static final String DATASET_QUEUE_NAME = "queue/B";
    protected static final String PROFILES_QUEUE_NAME = "queue/A";


//////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * sendJMSMessage not surprisingly sends a JMS message to the given
     * queueName on the local JBoss server.  This is used to contact a
     * MDB.
     *
     * @param queueName name of the local queue you want to send a message
     * to.
     * @param mapMessageValues an optional Map of values which become
     * the payload for the JMS message.
     * @throws NamingException is thrown when we are unable to locate
     * the queue named in the invocation.
     * @throws JMSException is thrown for messaging errors.
     */
    protected void sendJMSMessage ( String queueName, Map mapMessageValues )
            throws NamingException, JMSException {

        QueueConnection queueConnection = null;

        try {
            Context context = new InitialContext();

            // Get the connection factory
            QueueConnectionFactory queueFactory =
                (QueueConnectionFactory)context.lookup("ConnectionFactory");


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
        finally {
            queueConnection.close();
        }

    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
