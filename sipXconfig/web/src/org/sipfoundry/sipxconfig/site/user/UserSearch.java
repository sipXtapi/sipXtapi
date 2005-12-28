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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.search.BeanAdaptor;
import org.sipfoundry.sipxconfig.search.SearchManager;

public abstract class UserSearch extends BaseComponent {

    public static final String COMPONENT = "UserSearch";

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract List getUsers();

    public abstract void setUsers(List users);

    public abstract CoreContext getCoreContext();

    public abstract SearchManager getSearchManager();

    public abstract boolean getSimpleSearch();

    public abstract String getQuery();

    public void search(IRequestCycle cycle_) {
        List results = null;
        String query = getQuery();

        if (getSimpleSearch() && StringUtils.isNotBlank(query)) {
            BeanAdaptor.IdentityToBean identityToBean = new BeanAdaptor.IdentityToBean(getCoreContext());
            results = getSearchManager().search(User.class, query, identityToBean);
        } else {
            results = getCoreContext().loadUserByTemplateUser(getUser());
        }

        // keep original collection, reference has already been given to other
        // components.
        setUsers(new ArrayList(results));
        // record success message
        IValidationDelegate delegate = TapestryUtils.getValidator((AbstractComponent) getPage());
        if (delegate instanceof SipxValidationDelegate) {
            SipxValidationDelegate validator = (SipxValidationDelegate) delegate;
            String msg = getMessages().format("msg.found", new Integer(results.size()));                
            validator.recordSuccess(msg);
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
