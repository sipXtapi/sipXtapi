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
package org.sipfoundry.sipxconfig.components;

import javax.servlet.http.HttpServletRequest;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.RedirectException;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageValidateListener;

public abstract class Border extends BaseComponent implements PageValidateListener {
    /**
     * When true - only SUPER can see the pages, when false END_USER is accepted as well as admin
     */
    public abstract boolean isRestricted();

    public void pageValidate(PageEvent event) {
        HttpServletRequest request = event.getRequestCycle().getRequestContext().getRequest();
        if (!checkAuthorization(request)) {
            throw new RedirectException("/sipxconfig/login_error.html");
        }
    }

    /**
     * True is user is allowed to see the page, false otherwise
     * 
     * @param request servlet request used to check user's role
     */
    boolean checkAuthorization(HttpServletRequest request) {
        if (request.isUserInRole("SUPER")) {
            return true;
        }
        if (!isRestricted() && request.isUserInRole("END_USER")) {
            return true;
        }
        return false;
    }
}
