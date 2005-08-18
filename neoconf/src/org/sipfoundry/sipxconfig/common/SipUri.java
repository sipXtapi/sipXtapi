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
package org.sipfoundry.sipxconfig.common;

import java.text.MessageFormat;

import org.apache.commons.lang.StringUtils;

public class SipUri {
    private String m_uri;

    public SipUri(User user, String domainName) {
        m_uri = format(user, domainName);
    }

    public SipUri(String userName, String domain, int port) {
        String uri = format(userName, domain, port);
        m_uri = uri;
    }

    public SipUri(String userName, String domain, boolean quote) {
        m_uri = format(userName, domain, quote);
    }

    public static String format(User user, String domainName) {
        StringBuffer uri = new StringBuffer();
        boolean needsWrapping = false;

        String displayName = user.getDisplayName();
        if (org.apache.commons.lang.StringUtils.isNotBlank(displayName)) {
            needsWrapping = true;
            uri.append(displayName);
        }

        String uriProper = format(user.getUserName(), domainName, needsWrapping);
        uri.append(uriProper);
        return uri.toString();        
    }
    
    public static String format(String userName, String domainName, int port) {
        Object[] params = {
            userName, domainName, Integer.toString(port)
        };
        String uri = MessageFormat.format("sip:{0}@{1}:{2}", params);
        return uri;
    }
    
    public static String format(String userName, String domain, boolean quote) {
        Object[] params = {
            quote ? "<" : StringUtils.EMPTY, userName, domain, quote ? ">" : StringUtils.EMPTY
        };
        return MessageFormat.format("{0}sip:{1}@{2}{3}", params);
    }

    public String toString() {
        return m_uri;
    }
}
