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
package org.sipfoundry.sipxconfig.site.skin;

import java.util.Locale;

import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.services.ComponentMessagesSource;

/**
 * Skins can load their own resource strings depending on client locale of web browser
 */
public class SkinMessagesSource implements ComponentMessagesSource {
    
    private ComponentMessagesSource m_systemMessagesSource;
    private SkinControl m_skin;

    public void setSkin(SkinControl skin) {
        m_skin = skin;
    }

    public void setSystemMessagesSource(ComponentMessagesSource systemMessagesSource) {
        m_systemMessagesSource = systemMessagesSource;
    }

    public Messages getMessages(IComponent component) {
        return new SkinMessages(component.getId(), m_systemMessagesSource.getMessages(component), 
                component.getPage().getLocale(), m_skin);
    }
    
    static class SkinMessages extends AbstractMessages {
        private Messages m_systemMessages;
        private SkinControl m_skin;
        private Locale m_locale;
        private String m_componentId;
        SkinMessages(String componentId, Messages systemMessages, Locale locale, SkinControl skin) {
            m_systemMessages = systemMessages;
            m_locale = locale;
            m_skin = skin;
            m_componentId = componentId;
        }
        @Override
        protected String findMessage(String key) {
            String defaultMessage = m_systemMessages.getMessage(key);
            return m_skin.getLocalizeString(m_componentId, key, m_locale, defaultMessage);
        }
        @Override
        protected Locale getLocale() {
            return m_locale;
        }
    }
}
