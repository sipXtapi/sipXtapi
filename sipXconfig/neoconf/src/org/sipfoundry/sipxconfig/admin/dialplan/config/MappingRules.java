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
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Special type of mappingrules document with a single host match matching standard SIPx hosts
 */
public class MappingRules extends XmlFile implements ConfigFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00";
    private static final String[] HOSTS = {
        "${SIPXCHANGE_DOMAIN_NAME}", "${MY_FULL_HOSTNAME}", "${MY_HOSTNAME}", "${MY_IP_ADDR}"
    };

    private Document m_doc;
    private Element m_hostMatch;

    public MappingRules() {
        this(NAMESPACE);
    }

    protected MappingRules(String namespace) {
        m_doc = FACTORY.createDocument();
        QName mappingsName = FACTORY.createQName("mappings", namespace);
        Element mappings = m_doc.addElement(mappingsName);
        m_hostMatch = createFirstHostMatch(mappings);
    }

    /**
     * @param mappings root element of the mappingrules document
     */
    private Element createFirstHostMatch(Element mappings) {
        Element hostMatch1 = mappings.addElement("hostMatch");
        for (int i = 0; i < HOSTS.length; i++) {
            String host = HOSTS[i];
            Element hostPattern = hostMatch1.addElement("hostPattern");
            hostPattern.setText(host);
        }
        return hostMatch1;
    }

    public Element getFirstHostMatch() {
        return m_hostMatch;
    }

    public Document getDocument() {
        return m_doc;
    }

    public void generate(IDialingRule rule) {
        if (rule.isInternal()) {
            generateRule(rule);
        }
    }

    protected final void generateRule(IDialingRule rule) {
        Element hostMatch = getFirstHostMatch();
        Element userMatch = hostMatch.addElement("userMatch");
        String[] patterns = rule.getPatterns();
        for (int i = 0; i < patterns.length; i++) {
            String pattern = patterns[i];
            Element userPattern = userMatch.addElement("userPattern");
            userPattern.setText(pattern);
        }
        Element permissionMatch = userMatch.addElement("permissionMatch");
        if (rule.isInternal()) {
            List permissions = rule.getPermissions();
            for (Iterator i = permissions.iterator(); i.hasNext();) {
                Permission permission = (Permission) i.next();
                Element permissionElement = permissionMatch.addElement("permission");
                permissionElement.setText(permission.getName());
            }
        }
        Transform[] transforms = rule.getTransforms();
        for (int i = 0; i < transforms.length; i++) {
            Transform transform = transforms[i];
            transform.addToParent(permissionMatch);
        }
    }
}
