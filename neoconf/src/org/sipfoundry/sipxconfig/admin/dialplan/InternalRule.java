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

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule.Type;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * InternalRule
 */
public class InternalRule extends DialingRule {
    private static final String DEFAULT_VMAIL_PREFIX = "8";
    private static final int DEFAULT_LOCAL_EXT_LEN = 3;

    private String m_voiceMailPrefix = DEFAULT_VMAIL_PREFIX;
    private Integer m_localExtensionLen = new Integer(DEFAULT_LOCAL_EXT_LEN);
    private String m_autoAttendant = "100";
    private String m_voiceMail = "101";

    
    public String[] getPatterns() {
        return null;
    }

    public Transform[] getTransforms() {
        return null;
    }
    
    public Type getType() {
        return Type.INTERNAL;
    }
    
    public String getAutoAttendant() {
        return m_autoAttendant;
    }
    public void setAutoAttendant(String autoAttendant) {
        m_autoAttendant = autoAttendant;
    }
    public Integer getLocalExtensionLen() {
        return m_localExtensionLen;
    }
    public void setLocalExtensionLen(Integer localExtensionLen) {
        m_localExtensionLen = localExtensionLen;
    }
    public String getVoiceMail() {
        return m_voiceMail;
    }
    public void setVoiceMail(String voiceMail) {
        m_voiceMail = voiceMail;
    }
    public String getVoiceMailPrefix() {
        return m_voiceMailPrefix;
    }
    public void setVoiceMailPrefix(String voiceMailPrefix) {
        m_voiceMailPrefix = voiceMailPrefix;
    }
}
