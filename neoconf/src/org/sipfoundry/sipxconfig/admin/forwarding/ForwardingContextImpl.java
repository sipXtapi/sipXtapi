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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.common.User;
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
        getHibernateTemplate().update(callSequence);
        // notify profilegenerator if jms has been configured
        if (null != m_jms) {
            m_jms.send(new GenerateAliasesMessage());
        }
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public CallSequence getCallSequenceForUserId(Integer userId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (CallSequence) hibernate.load(CallSequence.class, userId);
    }

    public Ring getRing(Integer id) {
        HibernateTemplate hibernate = getHibernateTemplate();
        return (Ring) hibernate.load(Ring.class, id);
    }

    public List getForwardingAliases() {
        List aliases = new ArrayList();
        Organization organization = m_coreContext.loadRootOrganization();
        String ringsForUser = "from CallSequence cs where cs.user.organization = :organization";
        List sequences = getHibernateTemplate().findByNamedParam(ringsForUser, "organization", organization);
        //List sequences = getHibernateTemplate().loadAll(CallSequence.class);
        for (Iterator i = sequences.iterator(); i.hasNext();) {
            CallSequence sequence = (CallSequence) i.next();
            aliases.addAll(sequence.generateAliases());
        }
        return aliases;
    }

    private static class GenerateAliasesMessage implements MessageCreator {
        private static final String PARAM_NAME = "datasettype"; 
        private static final String TYPE_ALIAS = "aliases"; 
        /**
         * Sends generateAliases message
         * 
         * @param session the JMS session
         * @return the message to be sentt
         * @throws javax.jms.JMSException if thrown by JMS API methods
         */
        public Message createMessage(Session session) throws JMSException {
            MapMessage message = session.createMapMessage();
            message.setString(PARAM_NAME, TYPE_ALIAS);
            return message;
        }
    }

    public void setJms(JmsOperations jms) {
        m_jms = jms;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }    
}
