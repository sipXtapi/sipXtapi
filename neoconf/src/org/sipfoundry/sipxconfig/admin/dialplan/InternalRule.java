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

import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * InternalRule
 */
public class InternalRule extends DialingRule {
    private static final String DEFAULT_VMAIL_PREFIX = "8";
    private static final int DEFAULT_LOCAL_EXT_LEN = 3;

    private String m_voiceMailPrefix = DEFAULT_VMAIL_PREFIX;
    private int m_localExtensionLen = DEFAULT_LOCAL_EXT_LEN;
    private AutoAttendant m_autoAttendant;
    private String m_voiceMail = "101";

    public String[] getPatterns() {
        return null;
    }

    public Transform[] getTransforms() {
        return null;
    }

    public DialingRuleType getType() {
        return DialingRuleType.INTERNAL;
    }

    public AutoAttendant getAutoAttendant() {
        return m_autoAttendant;
    }

    public void setAutoAttendant(AutoAttendant autoAttendant) {
        m_autoAttendant = autoAttendant;
    }

    public int getLocalExtensionLen() {
        return m_localExtensionLen;
    }

    public void setLocalExtensionLen(int localExtensionLen) {
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

    public void appendToGenerationRules(List rules) {
        if (!isEnabled()) {
            return;
        }
        boolean generateVoiceMailRules = StringUtils.isNotBlank(m_voiceMail);
        if (m_autoAttendant != null) {
            MappingRule operator = new MappingRule.Operator(getName(), m_autoAttendant);
            operator.setDescription(getDescription());
            rules.add(operator);
        }
        if (generateVoiceMailRules) {
            MappingRule voicemail = new MappingRule.Voicemail(m_voiceMail);
            voicemail.setDescription(getDescription());
            rules.add(voicemail);
        }
        if (StringUtils.isNotBlank(m_voiceMailPrefix)) {
            MappingRule transfer = new MappingRule.VoicemailTransfer(m_voiceMailPrefix,
                    m_localExtensionLen);
            transfer.setDescription(getDescription());
            rules.add(transfer);
        }
        if (generateVoiceMailRules) {
            // pass -1 to generate fallback rule that matches any extension
            MappingRule fallback = new MappingRule.VoicemailFallback(-1);
            fallback.setDescription(getDescription());
            rules.add(fallback);
        }
    }

    public boolean isInternal() {
        return true;
    }
}
