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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import javax.jms.JMSException;
import javax.jms.MapMessage;
import javax.jms.Message;
import javax.jms.Session;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.legacy.LegacyContext;
import org.springframework.jms.core.JmsOperations;
import org.springframework.jms.core.MessageCreator;
import org.springframework.orm.hibernate.HibernateTemplate;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * ForwardingContextImpl
 * 
 */
public class ForwardingContextImpl extends HibernateDaoSupport implements ForwardingContext {
    private JmsOperations m_jms;
    private CoreContext m_coreContext;
    private LegacyContext m_legacyContext;

    /**
     * Looks for a call sequence associated with a given user.
     * 
     * This version just assumes that CallSequence id is the same as user id. More general
     * implementation would run a query. <code>
     *      String ringsForUser = "from CallSequence cs where cs.user = :user";
     *      hibernate.findByNamedParam(ringsForUser, "user", user);
     * </code>
     * 
     * @param user for which CallSequence object is retrieved
     */
    public CallSequence getCallSequenceForUser(User user) {
        return getCallSequenceForUserId(user.getId());
    }

    public void saveCallSequence(CallSequence callSequence) {
        saveCallSequence(callSequence, true);
    }

    private void saveCallSequence(CallSequence callSequence, boolean notify) {
        getHibernateTemplate().update(callSequence);
        // notify profilegenerator if jms has been configured
        if (notify && null != m_jms) {
            m_jms.send(new GenerateMessage(GenerateMessage.TYPE_ALIAS));
            m_jms.send(new GenerateMessage(GenerateMessage.TYPE_AUTH_EXCEPTIONS));
        }
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public CallSequence getCallSequenceForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (CallSequence) hibernate.load(CallSequence.class, userId);
    }

    public void removeCallSequenceForUserId(Integer userId, boolean notify) {
        CallSequence callSequence = getCallSequenceForUserId(userId);
        callSequence.clear();
        saveCallSequence(callSequence, notify);
    }

    public Ring getRing(Integer id) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (Ring) hibernate.load(Ring.class, id);
    }

    public List getForwardingAliases() {
        List aliases = new ArrayList();
        List sequences = loadAllCallSequences();
        for (Iterator i = sequences.iterator(); i.hasNext();) {
            CallSequence sequence = (CallSequence) i.next();
            aliases.addAll(sequence.generateAliases());
        }
        return aliases;
    }

    public List getForwardingAuthExceptions() {
        List aliases = new ArrayList();
        List sequences = loadAllCallSequences();
        for (Iterator i = sequences.iterator(); i.hasNext();) {
            CallSequence sequence = (CallSequence) i.next();
            User user = sequence.getUser();
            if (m_legacyContext.checkUserPermission(user, Permission.FORWARD_CALLS_EXTERNAL)) {
                aliases.addAll(sequence.generateAuthExceptions());
            }
        }
        return aliases;
    }

    /**
     * Loads call sequences for all uses in current root organization
     * 
     * @return list of CallSequence objects
     */
    private List loadAllCallSequences() {
        Organization organization = m_coreContext.loadRootOrganization();
        String ringsForUser = "from CallSequence cs where cs.user.organization = :organization";
        List sequences = getHibernateTemplate().findByNamedParam(ringsForUser, "organization",
                organization);
        return sequences;
    }

    private static class GenerateMessage implements MessageCreator {
        private static final String PARAM_NAME = "datasettype";
        private static final String TYPE_ALIAS = "aliases";
        private static final String TYPE_AUTH_EXCEPTIONS = "authexceptions";

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

    public void setJms(JmsOperations jms) {
        m_jms = jms;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setLegacyContext(LegacyContext legacyContext) {
        m_legacyContext = legacyContext;
    }
}
