/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.Session;

import org.springframework.jms.core.MessageCreator;

public class GenerateMessage implements MessageCreator {
    public static final String PARAM_NAME = "datasettype";
    public static final String TYPE_ALIAS = "aliases";
    public static final String TYPE_AUTH_EXCEPTIONS = "authexceptions";

    private String m_type;

    /**
     * @param type types of the data set to be generated as a result of sending of this
     *        message
     */
    public GenerateMessage(String type) {
        // TODO Auto-generated constructor stub
        m_type = type;
    }

    /**
     * Sends generateAliases message
     * 
     * @param session the JMS session
     * @return the message to be sentt
     * @throws javax.jms.JMSException if thrown by JMS API methods
     */
    public Message createMessage(Session session) throws JMSException {
        MapMessage message = session.createMapMessage();
        message.setString(PARAM_NAME, m_type);
        return message;
    }
}