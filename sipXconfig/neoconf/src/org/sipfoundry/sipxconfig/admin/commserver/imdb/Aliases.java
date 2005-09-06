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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Collection;
import java.util.Iterator;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class Aliases extends DataSetGenerator {

    private AliasProvider m_aliasProvider;

    public Aliases() {
    }

    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    protected void addItems(Element items) {
        addAliases(items, m_aliasProvider.getAliasMappings());
    }

    void addAliases(Element items, Collection aliases) {
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            AliasMapping alias = (AliasMapping) i.next();
            Element item = addItem(items);
            Element identity = item.addElement("identity");
            identity.setText(alias.getIdentity());
            Element contact = item.addElement("contact");
            contact.setText(alias.getContact());
        }
    }
    
    public void setAliasProvider(AliasProvider aliasProvider) {
        m_aliasProvider = aliasProvider;
    }
}
