/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

public class CallerAliases extends DataSetGenerator {
    private GatewayContext m_gatewayContext;

    protected void addItems(Element items) {
        // FIXME: use only gateways that are used in dialplan...
        List<Gateway> gateways = m_gatewayContext.getGateways();
        List<User> users = getCoreContext().loadUsers();
        String sipDomain = getSipDomain();

        for (Gateway gateway : gateways) {
            String gatewayAddr = gateway.getAddress();
            // add default entry for the gateway
            String defaultCallerAlias = gateway.getDefaultCallerAlias();

            if (StringUtils.isNotEmpty(defaultCallerAlias)) {
                String callerAliasUri = SipUri.format(defaultCallerAlias, sipDomain, false);
                addItem(items, gatewayAddr, callerAliasUri);
            } else {
                // add per-user entries
                for (User user : users) {
                    String externalNumber = user.getExternalNumber();
                    if (StringUtils.isNotEmpty(externalNumber)) {
                        String callerAliasUri = SipUri.format(user.getDisplayName(), externalNumber, sipDomain);
                        String identity = AliasMapping.createUri(user.getUserName(), sipDomain);
                        addItem(items, gatewayAddr, callerAliasUri, identity);
                    }
                }
            }
        }
    }

    private Element addItem(Element items, String domain, String alias, String identity) {
        Element item = addItem(items);
        if (identity != null) {
            item.addElement("identity").setText(identity);
        }
        item.addElement("target_domain").setText(domain);
        item.addElement("caller_alias").setText(alias);
        return item;
    }

    private Element addItem(Element items, String domain, String alias) {
        return addItem(items, domain, alias, null);
    }

    protected DataSet getType() {
        return DataSet.CALLER_ALIAS;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }
}
