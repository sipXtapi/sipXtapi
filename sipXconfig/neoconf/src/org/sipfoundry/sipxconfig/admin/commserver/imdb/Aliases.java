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

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;

public class Aliases extends DataSetGenerator {

    private ForwardingContext m_forwardingContext;

    private CallGroupContext m_callGroupContext;

    public Aliases() {
    }

    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    protected void addItems(Element items) {
        List forwardingAliases = m_forwardingContext.getForwardingAliases();
        List callGroupAliases = m_callGroupContext.getAliases();
        forwardingAliases.addAll(callGroupAliases);
        for (Iterator i = forwardingAliases.iterator(); i.hasNext();) {
            AliasMapping alias = (AliasMapping) i.next();
            Element item = addItem(items);
            Element identity = item.addElement("identity");
            identity.setText(alias.getIdentity());
            Element contact = item.addElement("contact");
            contact.setText(alias.getContact());
        }
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
