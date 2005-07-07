/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.resource;


public class ResourceContext {

    private ResourceTranslator m_translator;
    
    private String m_uri;

    public ResourceTranslator getTranslator() {
        return m_translator;
    }

    public void setTranslator(ResourceTranslator translator) {
        m_translator = translator;
    }

    public String getUri() {
        return m_uri;
    }

    public void setUri(String uri) {
        m_uri = uri;
    }
}

