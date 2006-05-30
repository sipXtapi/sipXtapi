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
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LdapImportPreview extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "LdapImportPreview";

    public abstract LdapImportManager getLdapImportManager();

    public abstract void setUser(User user);

    public abstract User getUser();

    public abstract void setGroupsString(String groups);

    public void pageBeginRender(PageEvent event) {
        if (getUser() == null) {
            setUser(new User());
        }

        if (!event.getRequestCycle().isRewinding()) {
            importExampleUser();
        }
    }

    public void importExampleUser() {
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator(getPage());
        try {
            Collection<String> groupNames = new ArrayList<String>();
            getLdapImportManager().getExample(getUser(), groupNames);
            String groupsString = StringUtils.join(groupNames.iterator(), " ");
            setGroupsString(groupsString);

            validator.recordSuccess(getMessages().getMessage("msg.success"));

        } catch (UserException e) {
            validator.record(new ValidatorException(e.getMessage()));
        }
    }

    public String ok() {
        return LdapImport.PAGE;
    }
}
