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
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.HashSet;
import java.util.Set;

/**
 * DialPlan - settings for configuring dial plan
 */
public class DialPlan {
    public static final String ACTIVE = "Active";
    public static final String INACTIVE = "Inactive";
    private static int s_id = 1;
    private static final int DEFAULT_LOCAL_EXT_LEN = 3;
    private static final String DEFAULT_PSTN_PREFIX = "9";
    private static final String DEFAULT_VMAIL_PREFIX = "8";
    private static final String DEFAULT_DID = "";

    // dial plan attributes
    private Integer m_id;
    private String m_name;
    private Integer m_localExtensionLen = new Integer(DEFAULT_LOCAL_EXT_LEN);
    private String m_autoAttendant = "100";
    private String m_voiceMail = "101";
    private String m_did = DEFAULT_DID;
    private String m_pstnPrefix = DEFAULT_PSTN_PREFIX;
    private String m_emergencyNumber = "911";
    private String m_internationalPrefix = "011";
    private String m_voiceMailPrefix = DEFAULT_VMAIL_PREFIX;
    private String m_longDistancePrefix = "1";
    private String m_status = INACTIVE;
    private String m_description = new String();
    private Set m_gateways = new HashSet();
    private Set m_emergencyGateways = new HashSet();

    public DialPlan() {
        m_id = new Integer(s_id++);
    }

    public DialPlan(String name, int localExtensionLen) {
        m_name = name;
        m_localExtensionLen = new Integer(localExtensionLen);
    }

    public String getAutoAttendant() {
        return m_autoAttendant;
    }

    public void setAutoAttendant(String autoAttendant) {
        m_autoAttendant = autoAttendant;
    }

    public String getDid() {
        return m_did;
    }

    public void setDid(String did) {
        m_did = did;
    }

    public String getEmergencyNumber() {
        return m_emergencyNumber;
    }

    public void setEmergencyNumber(String emergencyNumber) {
        m_emergencyNumber = emergencyNumber;
    }

    public String getInternationalPrefix() {
        return m_internationalPrefix;
    }

    public void setInternationalPrefix(String internationalPrefix) {
        m_internationalPrefix = internationalPrefix;
    }

    public String getLongDistancePrefix() {
        return m_longDistancePrefix;
    }

    public void setLongDistancePrefix(String longDistancePrefix) {
        m_longDistancePrefix = longDistancePrefix;
    }

    public String getPstnPrefix() {
        return m_pstnPrefix;
    }

    public void setPstnPrefix(String pstnPrefix) {
        m_pstnPrefix = pstnPrefix;
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

    public Integer getLocalExtensionLen() {
        return m_localExtensionLen;
    }

    public void setLocalExtensionLen(Integer localExtensionLen) {
        m_localExtensionLen = localExtensionLen;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public Set getGateways() {
        return m_gateways;
    }

    public Set getEmergencyGateways() {
        return m_emergencyGateways;
    }

    private Set getGatewaySet(boolean emergency) {
        return emergency ? m_emergencyGateways : m_gateways;
    }

    public boolean addGateway(Gateway gateway, boolean emergency) {
        return getGatewaySet(emergency).add(gateway);
    }

    public boolean removeGateway(Gateway gateway, boolean emergency) {
        return getGatewaySet(emergency).remove(gateway);
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getStatus() {
        return m_status;
    }

    public void setStatus(String status) {
        m_status = status;
    }

    public boolean equals(Object o) {
        DialPlan other = (DialPlan) o;
        if (other == null) {
            return false;
        }
        return m_id.equals(other.m_id);
    }

    public int hashCode() {
        return m_id.hashCode();
    }

    public Integer getId() {
        return m_id;
    }

    public void setId(Integer id) {
        m_id = id;
    }
}
