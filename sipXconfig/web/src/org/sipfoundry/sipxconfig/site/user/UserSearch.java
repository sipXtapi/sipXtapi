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
package org.sipfoundry.sipxconfig.site.user;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public abstract class UserSearch extends BaseComponent {

    public static final String COMPONENT = "UserSearch";

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract List getUsers();

    public abstract void setUsers(List users);

    public abstract CoreContext getCoreContext();

    public void search(IRequestCycle cycle_) {
        User user = getUser();
        if (user != null) {
            List results = getCoreContext().loadUserByTemplateUser(getUser());
            // keep original collection, reference has already been given to other
            // components.
            setUsers(new ArrayList(results));
        }
    }

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        if (getUsers() == null) {
            setUsers(Collections.EMPTY_LIST);
        }
        if (getUser() == null) {
            setUser(new User());
        }

        super.renderComponent(writer, cycle);
    }
}
