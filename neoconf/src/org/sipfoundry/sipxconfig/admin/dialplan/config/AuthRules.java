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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.Iterator;
import java.util.List;

import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;

import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Authorization rule generator.
 * 
 * One dialing rules corresponds to one hostMatch element. All gateways end up
 * in hostPatterns, all dialing patterns are put in userPatterns. Permissions
 * are added to the resulting permissions match code.
 * 
 * <code>
 * <hostMatch>
 *    <hostPattern>gateway addresses</hostPattern>
 *    <userMatch>
 *      <userPattern>sos</userPattern> 
 *      <permissionMatch>name of the permission</permissionMatch>
 *    </userMatch>
 * </hostMatch>
 * </code>
 * 
 */
public class AuthRules {
    private static final DocumentFactory FACTORY = DocumentFactory.getInstance();

    private Document m_doc;

    public AuthRules() {
        m_doc = FACTORY.createDocument();
        m_doc.addElement("mappings");
    }

    void generate(IDialingRule rule) {
        Element mappings = m_doc.getRootElement();
        Element hostMatch = mappings.addElement("hostMatch");
        hostMatch.addComment(rule.getName());
        List gateways = rule.getGateways();
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            Element hostPattern = hostMatch.addElement("hostPattern");
            hostPattern.setText(gateway.getAddress());
        }
        Element userMatch = hostMatch.addElement("userMatch");
        String[] patterns = rule.getPatterns();
        for (int i = 0; i < patterns.length; i++) {
            String pattern = patterns[i];
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(pattern);
        }
        Permission[] permissions = rule.getPermissions();
        Element permissionMatch = userMatch.addElement("permissionMatch");
        for (int i = 0; i < permissions.length; i++) {
            Permission permission = permissions[i];
            Element pelement = permissionMatch.addElement("permission");
            pelement.setText(permission.getName());
        }
    }

    public Document getDocument() {
        return m_doc;
    }
}
