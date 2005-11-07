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
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CollectionUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeContextImpl extends HibernateDaoSupport implements BeanFactoryAware,
        ConferenceBridgeContext {
    private static final String USER = "user";
    private static final String CONFERENCE = "conference";
    private static final String VALUE = "value";
    private static final String CONFERENCE_IDS_WITH_ALIAS = "conferenceIdsWithAlias";

    private CoreContext m_coreContext;

    private BeanFactory m_beanFactory;

    public List getBridges() {
        return getHibernateTemplate().loadAll(Bridge.class);
    }

    public void store(Bridge bridge) {
        getHibernateTemplate().saveOrUpdate(bridge);
    }

    public void store(Conference conference) {
        getHibernateTemplate().saveOrUpdate(conference);
    }

    public Bridge newBridge() {
        return (Bridge) m_beanFactory.getBean(Bridge.BEAN_NAME, Bridge.class);
    }

    public Conference newConference() {
        Conference conference = (Conference) m_beanFactory.getBean(Conference.BEAN_NAME,
                Conference.class);
        conference.generateAccessCodes();
        return conference;
    }

    private Participant newParticipant() {
        return (Participant) m_beanFactory.getBean(Participant.BEAN_NAME, Participant.class);
    }

    public void removeBridges(Collection bridgesIds) {
        List bridges = new ArrayList(bridgesIds.size());
        for (Iterator i = bridgesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            Bridge bridge = loadBridge(id);
            bridges.add(bridge);
        }
        getHibernateTemplate().deleteAll(bridges);
    }

    public void removeConferences(Collection conferencesIds) {
        Set bridges = new HashSet();
        for (Iterator i = conferencesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            Conference conference = loadConference(id);
            Bridge bridge = conference.getBridge();
            bridge.removeConference(conference);
            bridges.add(bridge);
        }
        getHibernateTemplate().saveOrUpdateAll(bridges);
    }

    public void removeParticipants(Collection participantsIds) {
        Set conferences = new HashSet();
        for (Iterator i = participantsIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            Participant participant = loadParticipant(id);
            Conference conference = participant.getConference();
            conference.removeParticipant(participant);
            conferences.add(conference);
        }
        getHibernateTemplate().saveOrUpdateAll(conferences);
    }

    public void addParticipantsToConference(Serializable conferenceId, Collection usersIds) {
        Conference conference = loadConference(conferenceId);
        for (Iterator i = usersIds.iterator(); i.hasNext();) {
            Integer userId = (Integer) i.next();
            User user = m_coreContext.loadUser(userId);
            Participant participant = getParticipant(conference, user);
            if (participant == null) {
                participant = newParticipant();
                participant.setUser(user);
                conference.insertParticipant(participant);
            }
        }
        getHibernateTemplate().update(conference);
    }

    private Participant getParticipant(Conference conference, User user) {
        String[] params = {
            CONFERENCE, USER
        };
        Object[] values = {
            conference, user
        };
        List participants = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "participantForConferenceAndUser", params, values);
        if (participants.isEmpty()) {
            return null;
        }
        return (Participant) participants.get(0);
    }

    public Bridge loadBridge(Serializable id) {
        return (Bridge) getHibernateTemplate().load(Bridge.class, id);
    }

    public Conference loadConference(Serializable id) {
        return (Conference) getHibernateTemplate().load(Conference.class, id);
    }

    public Participant loadParticipant(Serializable id) {
        return (Participant) getHibernateTemplate().load(Participant.class, id);
    }

    public List getAliases() {
        // TODO generate real aliases
        return Collections.EMPTY_LIST;
    }

    public void clear() {
        List bridges = getBridges();
        getHibernateTemplate().deleteAll(bridges);
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            List participantIds = getHibernateTemplate().findByNamedQueryAndNamedParam(
                    "participantIdForUser", USER, user);
            removeParticipants(participantIds);
        }
    }

    // trivial get/set
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public boolean isAliasInUse(String alias) {
        List confIds = getHibernateTemplate().findByNamedQueryAndNamedParam(
                CONFERENCE_IDS_WITH_ALIAS, VALUE, alias);
        return CollectionUtils.safeSize(confIds) > 0;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection ids = getHibernateTemplate().findByNamedQueryAndNamedParam(
                CONFERENCE_IDS_WITH_ALIAS, VALUE, alias);
        Collection bids = BeanId.createBeanIdCollection(ids, User.class);
        return bids;
    }

}
