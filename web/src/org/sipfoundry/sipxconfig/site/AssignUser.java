/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.EndpointAssignment;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.phone.User;

/**
 * Assign a user to a device
 */
public abstract class AssignUser extends AbstractPhonePage implements PageRenderListener {

    public void pageBeginRender(PageEvent eventTemp) {
        if (getAssignment() == null || getAssignment().getEndpoint() == null) {
            // unsure this is appropriate exception to throw 
            throw new IllegalArgumentException(
                    "internal error, endpoint or endpoint-assignment objects required");
        }
        
        User user = getAssignment().getUser();
        if (user != null && getUserId() == null) {
            setUserId(user.getDisplayId());
        }
    }

    public void save(IRequestCycle cycle) {
        PhoneDao dao = getPhoneContext().getPhoneDao();
        User user = dao.loadUserByDisplayId(getUserId());
        if (user == null) {
            // mark field invalid
            throw new IllegalArgumentException("user doesn't exist");
        }
        getAssignment().setUser(user);
        dao.storeEndpointAssignment(getAssignment());
        
        cycle.activate(PAGE_LIST_PHONES);
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(PAGE_LIST_PHONES);
    }

    public void setEndpoint(Endpoint endpoint) {
        EndpointAssignment assignment = new EndpointAssignment();
        assignment.setEndpoint(endpoint);
        setAssignment(assignment);
    }

    public abstract EndpointAssignment getAssignment();

    public abstract void setAssignment(EndpointAssignment assignment);

    public abstract String getUserId();

    public abstract void setUserId(String userId);
}