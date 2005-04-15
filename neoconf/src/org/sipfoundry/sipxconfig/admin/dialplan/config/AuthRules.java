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

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;
import org.sipfoundry.sipxconfig.admin.dialplan.Gateway;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Authorization rule generator.
 * 
 * One dialing rules corresponds to one hostMatch element. All gateways end up in hostPatterns,
 * all dialing patterns are put in userPatterns. Permissions are added to the resulting
 * permissions match code.
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
public class AuthRules extends XmlFile implements ConfigFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urlauth-00-00";
    private static final String NO_ACCESS_RULE = "Reject all other calls to the gateways"
            + " that are not handled by the earlier rules";

    private Document m_doc;
    private Set m_gateways = new HashSet();

    public AuthRules() {
        m_doc = FACTORY.createDocument();
        QName mappingsName = FACTORY.createQName("mappings", NAMESPACE);
        m_doc.addElement(mappingsName);
    }

    public void generate(IDialingRule rule) {
        List gateways = rule.getGateways();
        List permissions = rule.getPermissions();
        if (gateways.size() == 0) {
            // nothing to generate
            return;
        }
        Element mappings = m_doc.getRootElement();
        Element hostMatch = mappings.addElement("hostMatch");
        hostMatch.addComment(rule.getName());
        for (Iterator i = gateways.iterator(); i.hasNext();) {
            Gateway gateway = (Gateway) i.next();
            Element hostPattern = hostMatch.addElement("hostPattern");
            hostPattern.setText(gateway.getAddress());
            m_gateways.add(gateway);
        }
        Element userMatch = hostMatch.addElement("userMatch");
        String[] patterns = rule.getPatterns();
        for (int i = 0; i < patterns.length; i++) {
            String pattern = patterns[i];
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(pattern);
        }
        // even if no permission is specified (permission list is empty) we create empty element
        Element permissionMatch = userMatch.addElement("permissionMatch");
        for (Iterator i = permissions.iterator(); i.hasNext();) {
            Permission permission = (Permission) i.next();
            Element pelement = permissionMatch.addElement("permission");
            pelement.setText(permission.getName());
        }
    }

    public Document getDocument() {
        return m_doc;
    }

    void generateNoAccess(List gateways) {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setName(NO_ACCESS_RULE);
        rule.setGateways(gateways);
        rule.setPermissions(Collections.singletonList(Permission.NO_ACCESS));
        DialPattern matchAll = new DialPattern(".", 0);
        rule.setDialPatterns(Collections.singletonList(matchAll));
        generate(rule);
    }

    public void end() {
        generateNoAccess(new ArrayList(m_gateways));
    }
}
