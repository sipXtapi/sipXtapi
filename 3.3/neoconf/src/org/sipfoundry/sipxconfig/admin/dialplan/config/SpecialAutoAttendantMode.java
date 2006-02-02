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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class SpecialAutoAttendantMode extends XmlFile {
    private final AutoAttendant m_attendant;
    private final boolean m_enabled;

    public SpecialAutoAttendantMode(boolean enabled, AutoAttendant attendant) {
        m_enabled = enabled;
        m_attendant = attendant;
    }

    public Document getDocument() {
        Document document = FACTORY.createDocument();
        Element op = document.addElement("organizationprefs");
        op.addElement("specialoperation").setText(Boolean.toString(m_enabled));
        Element aa = op.addElement("autoattendant");
        if (m_enabled) {
            aa.setText(m_attendant.getSystemName());
        }

        return document;
    }

    public ConfigFileType getType() {
        return ConfigFileType.ORGANIZATION_PREFS;
    }
}
