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
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

public class CallerAliases extends DataSetGenerator {
    private GatewayContext m_gatewayContext;

    private String m_anonymousAlias;

    protected void addItems(Element items) {
        // FIXME: use only gateways that are used in dialplan...
        List<Gateway> gateways = m_gatewayContext.getGateways();
        List<User> users = getCoreContext().loadUsers();
        String sipDomain = getSipDomain();

        for (Gateway gateway : gateways) {
            String gatewayAddr = gateway.getAddress();
            // add default entry for the gateway
            GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();

            String defaultCallerAlias = gatewayInfo.getDefaultCallerAlias();
            if (StringUtils.isNotEmpty(defaultCallerAlias)) {
                String callerAliasUri = SipUri.format(defaultCallerAlias, sipDomain, false);
                addItem(items, gatewayAddr, callerAliasUri);
            }

            // only add user aliases is overwrite is not set
            if (gatewayInfo.isIgnoreUserInfo()) {
                continue;
            }
            // add per-user entries
            for (User user : users) {
                UserCallerAliasInfo info = new UserCallerAliasInfo(user);
                String externalNumber = getExternalNumber(gatewayInfo, info, user);
                if (externalNumber != null) {
                    String callerAliasUri = SipUri.format(user.getDisplayName(), externalNumber,
                            sipDomain);
                    String identity = AliasMapping.createUri(user.getUserName(), sipDomain);
                    addItem(items, gatewayAddr, callerAliasUri, identity);
                }
            }
        }
    }

    private String getExternalNumber(GatewayCallerAliasInfo gatewayInfo,
            UserCallerAliasInfo userInfo, User user) {
        if (userInfo.isAnonymous()) {
            return m_anonymousAlias;
        }
        String externalNumber = gatewayInfo.getTransformedNumber(user);
        if (externalNumber != null) {
            return externalNumber;
        }
        return userInfo.getExternalNumber();

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

    public void setAnonymousAlias(String anonymousAlias) {
        m_anonymousAlias = anonymousAlias;
    }
}
