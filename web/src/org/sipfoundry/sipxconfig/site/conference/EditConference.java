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
package org.sipfoundry.sipxconfig.site.conference;

import java.io.Serializable;
import java.util.Collection;

import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.site.user.SelectUsers;
import org.sipfoundry.sipxconfig.site.user.SelectUsersCallback;

public abstract class EditConference extends PageWithCallback implements PageRenderListener {
    public static final String PAGE = "EditConference";

    public abstract ConferenceBridgeContext getConferenceBridgeContext();

    public abstract Serializable getBridgeId();

    public abstract void setBridgeId(Serializable bridgeId);

    public abstract Serializable getConferenceId();

    public abstract void setConferenceId(Serializable id);

    public abstract Conference getConference();

    public abstract void setConference(Conference acdServer);

    public abstract boolean getChanged();

    public abstract Collection getNewUsersIds();

    public abstract void setNewUsersIds(Collection participantsIds);

    public void pageBeginRender(PageEvent event_) {
        addNewParticipants();
        if (getConference() != null) {
            return;
        }
        Conference conference = null;
        if (getConferenceId() != null) {
            conference = getConferenceBridgeContext().loadConference(getConferenceId());
        } else {
            conference = getConferenceBridgeContext().newConference();
        }
        setConference(conference);
    }

    protected void addNewParticipants() {
        Collection ids = getNewUsersIds();
        Serializable conferenceId = getConferenceId();
        if (ids != null && conferenceId != null) {
            getConferenceBridgeContext().addParticipantsToConference(conferenceId, ids);
        }
        setNewUsersIds(null);
    }

    public void apply(IRequestCycle cycle) {
        if (TapestryUtils.isValid(this)) {
            saveValid(cycle);
        }
    }

    private void saveValid(IRequestCycle cycle_) {
        Conference conference = getConference();
        if (conference.isNew()) {
            // associate with bridge
            Bridge bridge = getConferenceBridgeContext().loadBridge(getBridgeId());
            bridge.insertConference(conference);
            getConferenceBridgeContext().store(bridge);
            Integer id = conference.getId();
            setConferenceId(id);
        } else {
            getConferenceBridgeContext().store(conference);
        }
    }

    public void formSubmit(IRequestCycle cycle_) {
        if (getChanged()) {
            setConference(null);
        }
    }

    public void addParticipant(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        saveValid(cycle);
        SelectParticipantsCallback callback = new SelectParticipantsCallback(getBridgeId(),
                getConferenceId());
        SelectUsers selectUsersPage = (SelectUsers) cycle.getPage(SelectUsers.PAGE);
        selectUsersPage.setCallback(callback);
        selectUsersPage.setPrompt(getMessage("prompt.selectParticipants"));
        selectUsersPage.setTitle(getMessage("title.selectParticipants"));
        cycle.activate(selectUsersPage);
    }

    public void editParticipant(IRequestCycle cycle) {
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        // not sure if we need to edit participant
        LogFactory.getLog(getClass()).debug("Editing participants not implemented: " + id);
    }

    /**
     * Activate this page
     * 
     * @param cycle current cycle
     * @param callback usually page callback to return to activating page
     * @param bridgeId bridge identifier
     * @param conferenceId conference identifier - can be null for new conference to be added
     */
    public void activate(IRequestCycle cycle, ICallback callback, Serializable bridgeId,
            Serializable conferenceId) {
        setBridgeId(bridgeId);
        setConferenceId(conferenceId);
        setCallback(callback);
        cycle.activate(this);
    }

    private static class SelectParticipantsCallback extends SelectUsersCallback {
        private Serializable m_bridgeId;

        private Serializable m_conferenceId;

        public SelectParticipantsCallback(Serializable bridgeId, Serializable conferenceId) {
            super(PAGE);
            setIdsPropertyName("newUsersIds");
            m_bridgeId = bridgeId;
            m_conferenceId = conferenceId;
        }

        protected void beforeActivation(AbstractPage page) {
            EditConference editConferencePage = (EditConference) page;
            editConferencePage.setBridgeId(m_bridgeId);
            editConferencePage.setConferenceId(m_conferenceId);
        }
    }
}
