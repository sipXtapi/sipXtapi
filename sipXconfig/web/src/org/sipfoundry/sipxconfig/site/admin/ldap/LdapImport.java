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

import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LdapImport extends BasePage implements PageBeginRenderListener {

    public abstract LdapConnectionParams getConnectionParams();

    public abstract void setConnectionParams(LdapConnectionParams setConnectionParams);

    public abstract LdapImportManager getLdapImportManager();

    public abstract LdapManager getLdapManager();

    public void pageBeginRender(PageEvent event_) {
        if (getConnectionParams() == null) {
            setConnectionParams(getLdapManager().getConnectionParams());
        }
    }

    public void importLdap() {
        if (!TapestryUtils.isValid((AbstractPage) getPage())) {
            return;
        }
        // save new connection parameters
        getLdapManager().setConnectionParams(getConnectionParams());
        // and import
        getLdapImportManager().insert();
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator(this);
        validator.recordSuccess(getMessages().getMessage("msg.success"));
    }
}
