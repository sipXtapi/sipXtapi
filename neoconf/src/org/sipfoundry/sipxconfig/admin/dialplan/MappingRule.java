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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * MappingRule
 */
public class MappingRule extends DialingRule {
    protected static final String URL_PREFIX = "<sip:{digits}@{mediaserver};play={voicemail}";
    protected static final String OPERATOR_URL = URL_PREFIX
            + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant>";
    protected static final String VOICEMAIL_URL = URL_PREFIX
            + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dretrieve%26mailbox%3D{digits}>";
    protected static final String VOICEMAIL_TRANSFER_URL = "<sip:{vdigits}@{mediaserver};play={voicemail}"
            + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits}>";
    protected static final String VOICEMAIL_FALLBACK_URL = URL_PREFIX
            + "%2Fcgi-bin%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{digits}>;q=0.1";

    private String[] m_patterns;
    private String m_url;

    public MappingRule() {
        this(new String[] {}, StringUtils.EMPTY);
    }

    /**
     * @param patterns list of patterns
     * @param url
     */
    public MappingRule(String[] patterns, String url) {
        m_patterns = patterns;
        m_url = url;
        setEnabled(true);
    }

    public String[] getPatterns() {
        return m_patterns;
    }

    public Transform[] getTransforms() {
        UrlTransform transform = new UrlTransform();
        transform.setUrl(m_url);
        return new Transform[] {transform};
    }

    public Type getType() {
        return Type.MAPPING_RULE;
    }

    public void setPatterns(String[] patterns) {
        m_patterns = patterns;
    }

    public void setUrl(String url) {
        m_url = url;
    }

    // specialized classes
    public static class Operator extends MappingRule {
        public Operator(String operator) {
            setPatterns(new String[] {"operator", operator, "0"});
            setUrl(MappingRule.OPERATOR_URL);
        }
    }

    public static class VoicemailFallback extends MappingRule {
        public VoicemailFallback(int extensionLen) {
            DialPattern pattern = new DialPattern(StringUtils.EMPTY, extensionLen);
            setPatterns(new String[] {pattern.calculatePattern()});
            setUrl(VOICEMAIL_FALLBACK_URL);
        }

        public List getPermissions() {
            List perms = new ArrayList();
            perms.add(Permission.VOICEMAIL);
            return perms;
        }
    }

    public static class Voicemail extends MappingRule {
        public Voicemail(String voiceMail) {
            setPatterns(new String[] {voiceMail});
            setUrl(VOICEMAIL_URL);
        }

    }

    public static class VoicemailTransfer extends MappingRule {
        public VoicemailTransfer(String prefix, int extensionLen) {
            DialPattern pattern = new DialPattern(prefix, extensionLen);
            setPatterns(new String[] {pattern.calculatePattern()});
            setUrl(VOICEMAIL_TRANSFER_URL);
        }
    }
}
