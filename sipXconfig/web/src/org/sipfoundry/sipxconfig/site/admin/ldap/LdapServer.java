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
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LdapServer extends BasePage implements PageBeginRenderListener {

    public abstract LdapConnectionParams getConnectionParams();

    public abstract void setConnectionParams(LdapConnectionParams setConnectionParams);

    public abstract AttrMap getAttrMap();

    public abstract void setAttrMap(AttrMap attrMap);

    public abstract LdapImportManager getLdapImportManager();

    public abstract LdapManager getLdapManager();

    public abstract String getStage();

    public abstract void setStage(String stage);

    public void pageBeginRender(PageEvent event_) {
        if (getConnectionParams() == null) {
            LdapManager ldapManager = getLdapManager();
            setConnectionParams(ldapManager.getConnectionParams());
            setAttrMap(ldapManager.getAttrMap());
        }

        if (getStage() == null) {
            setStage("connection");
        }
    }

    public void applyConnectionParams() {
        if (!TapestryUtils.isValid((AbstractPage) getPage())) {
            return;
        }
        // save new connection params
        getLdapManager().setConnectionParams(getConnectionParams());        
        setStage("attrs");
    }

    public String applyAttrMap() {
        if (!TapestryUtils.isValid((AbstractPage) getPage())) {
            return null;
        }
        getLdapManager().setAttrMap(getAttrMap());        
        return LdapImport.PAGE;
    }
}
