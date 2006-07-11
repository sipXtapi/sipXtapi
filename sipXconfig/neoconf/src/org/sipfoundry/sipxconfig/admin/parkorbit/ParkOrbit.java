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
package org.sipfoundry.sipxconfig.admin.parkorbit;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.NamedObject;

public class ParkOrbit extends BackgroundMusic implements NamedObject {
    private String m_name;
    private String m_extension;
    private String m_description;

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public AliasMapping generateAlias(String dnsDomain, String orbitServer) {
        String identity = AliasMapping.createUri(m_extension, dnsDomain);
        String contact = AliasMapping.createUri(m_extension, orbitServer);
        return new AliasMapping(identity, contact);
    }
}
