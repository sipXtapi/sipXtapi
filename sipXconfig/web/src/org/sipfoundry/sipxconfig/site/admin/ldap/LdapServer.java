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

import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.bulk.ldap.AttrMap;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapConnectionParams;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.Schema;
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

    public abstract Schema getSchema();

    public abstract void setSchema(Schema schema);

    public abstract Collection<String> getSelectedObjectClasses();

    public abstract void setSelectedObjectClasses(Collection<String> objectClasses);

    public abstract String[] getSelectedAttributes();

    public abstract void setSelectedAttributes(String[] selectedAttributes);

    public void pageBeginRender(PageEvent event_) {
        LdapManager ldapManager = getLdapManager();
        if (getConnectionParams() == null) {
            setConnectionParams(ldapManager.getConnectionParams());
        }

        if (getAttrMap() == null) {
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
        LdapConnectionParams connectionParams = getConnectionParams();
        AttrMap attrMap = getAttrMap();
        LdapManager ldapManager = getLdapManager();
        // check if we can connect to LDAP - throws user exception if there are any problems
        Schema schema = ldapManager.verify(connectionParams, attrMap);
        setSchema(schema);

        // save new connection params
        ldapManager.setConnectionParams(connectionParams);
        ldapManager.setAttrMap(attrMap);

        String objectClass = attrMap.getObjectClass();

        Collection<String> selectedObjectClasses = new ArrayList<String>(1);
        selectedObjectClasses.add(objectClass);
        setSelectedObjectClasses(selectedObjectClasses);

        setStage("objectClasses");
    }

    public void applyObjectClassesSelection() {
        Schema schema = getSchema();
        String[] attributesPool = schema.getAttributesPool(getSelectedObjectClasses());
        setSelectedAttributes(attributesPool);

        setStage("attrs");
    }

    public String applyAttrMap() {
        if (!TapestryUtils.isValid((AbstractPage) getPage())) {
            return null;
        }
        getLdapManager().setAttrMap(getAttrMap());
        return LdapImport.PAGE;
    }

    public IPropertySelectionModel getObjectClassesSelectionModel() {
        Collection<String> objectClasses = getSelectedObjectClasses();
        return new StringPropertySelectionModel(objectClasses.toArray(new String[objectClasses
                .size()]));
    }

    public IPropertySelectionModel getAttributesSelectionModel() {
        return new StringPropertySelectionModel(getSelectedAttributes());
    }
}
