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

import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

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

    public void pageBeginRender(PageEvent event_) {
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
        // do something to add them
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
}
