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
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;

public class SipUri {
    public static final String SIP_PREFIX = "sip:";
    public static final int DEFAULT_SIP_PORT = 5060;

    private static final Pattern EXTRACT_USER_RE = Pattern.compile("\\s*<?(?:sip:)?(.+?)@.+");
    private static final Pattern EXTRACT_FULL_USER_RE = Pattern
            .compile("(?:\"(.*)\")?\\s*<?(?:sip:)?(.+?)@.+");

    private String m_uri;

    public SipUri(User user, String domainName) {
        m_uri = format(user, domainName);
    }

    public SipUri(String userName, String domain, int port) {
        String uri = format(userName, domain, port);
        m_uri = uri;
    }

    public SipUri(String domain, int port) {
        String uri = format(domain, port);
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
            uri.append('"');
            uri.append(displayName);
            uri.append('"');
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

    public static String format(String domainName, int port) {
        Object[] params = {
            domainName, Integer.toString(port)
        };
        String uri = MessageFormat.format("sip:{0}:{1}", params);
        return uri;
    }
    
    public static int parsePort(String sPort, int defaultPort) {
        if (StringUtils.isBlank(sPort)) {
            return defaultPort;
        }
        int port = Integer.parseInt(sPort);
        return port;
    }
    
    public static String formatIgnoreDefaultPort(String userName, String domain, int port) {
        if (port == DEFAULT_SIP_PORT) {
            return format(userName, domain, false);
        }
        return format(userName, domain, port);
    }

    public static String formatIgnoreDefaultPort(String displayName, String userName, String domain, int port) {
        String baseUri = formatIgnoreDefaultPort(userName, domain, port);
        if (displayName == null) {
            return baseUri;
        }
        String uri = String.format("\"%s\"<%s>", displayName, baseUri);
        return uri;
    }

    public static String format(String userName, String domain, boolean quote) {
        Object[] params = {
            quote ? "<" : StringUtils.EMPTY, userName, domain, quote ? ">" : StringUtils.EMPTY
        };
        return MessageFormat.format("{0}sip:{1}@{2}{3}", params);
    }

    public static String normalize(String uri) {
        String result = uri.trim();
        if (result.startsWith(SIP_PREFIX)) {
            return result;
        }
        return SIP_PREFIX + result;
    }

    public static String extractUser(String uri) {
        Matcher matcher = EXTRACT_USER_RE.matcher(uri);
        if (matcher.matches()) {
            return matcher.group(1);
        }
        return null;
    }

    /**
     * Extract user id and optional user info
     * 
     * <!--
     *  
     * 154@example.org => 154 
     * sip:user@exampl.org => user 
     * "Full name"<sip:202@example.org> =>Full name - 202 
     * 
     * -->
     * 
     */
    public static String extractFullUser(String uri) {
        Matcher matcher = EXTRACT_FULL_USER_RE.matcher(uri);
        if (!matcher.matches()) {
            return null;
        }
        String fullName = matcher.group(1);
        String userId = matcher.group(2);

        if (fullName == null) {
            return userId;
        }
        return fullName + " - " + userId;
    }
    
    public static String format(String name, String domain, Map urlParams) {
        StringBuffer paramsBuffer = new StringBuffer();
        for (Iterator i = urlParams.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            paramsBuffer.append(';');
            paramsBuffer.append(entry.getKey());
            Object value = entry.getValue();
            if (value != null) {
                paramsBuffer.append('=');
                paramsBuffer.append(value);
            }
        }

        Object[] params = {
            name, domain, paramsBuffer
        };
        return MessageFormat.format("<sip:{0}@{1}{2}>", params);
    }

    public String toString() {
        return m_uri;
    }
}
