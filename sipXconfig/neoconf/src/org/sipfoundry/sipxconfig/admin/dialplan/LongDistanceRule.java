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
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.permission.Permission;

/**
 * LongDistanceRule
 */
public class LongDistanceRule extends DialingRule {
    private String m_pstnPrefix = StringUtils.EMPTY;
    private boolean m_pstnPrefixOptional;
    private String m_longDistancePrefix = StringUtils.EMPTY;
    private boolean m_longDistancePrefixOptional;
    private String m_areaCodes = StringUtils.EMPTY;
    private int m_externalLen;
    private Permission m_permission = Permission.LONG_DISTANCE_DIALING;

    public String[] getPatterns() {
        throw new UnsupportedOperationException("getPatterns not supported for LongDistance rule");
    }

    /**
     * Calculates list of dial patterns for a specified PSTN prefix, long distance prefix and area
     * code.
     * 
     * Each dial pattern describes the digit sequence that user dials in order to trigger this
     * rule.
     * 
     * @param areaCode single are code for which patterns will be generated
     * @return list of dial patterns objects
     */
    List<DialPattern> calculateDialPatterns(String areaCode) {
        int variableLenght = m_externalLen - areaCode.length();
        List<DialPattern> patterns = new ArrayList<DialPattern>();
        if (StringUtils.isNotBlank(m_pstnPrefix) && StringUtils.isNotBlank(m_longDistancePrefix)) {
            String prefix = m_pstnPrefix + m_longDistancePrefix + areaCode;
            patterns.add(new DialPattern(prefix, variableLenght));
        }
        if (m_pstnPrefixOptional && StringUtils.isNotBlank(m_longDistancePrefix)) {
            String prefix = m_longDistancePrefix + areaCode;
            patterns.add(new DialPattern(prefix, variableLenght));
        }
        if (StringUtils.isNotBlank(m_pstnPrefix) && m_longDistancePrefixOptional) {
            String prefix = m_pstnPrefix + areaCode;
            patterns.add(new DialPattern(prefix, variableLenght));
        }
        if (m_pstnPrefixOptional && m_longDistancePrefixOptional) {
            patterns.add(new DialPattern(areaCode, variableLenght));
        }
        return patterns;
    }

    /**
     * Calculates the call pattern - the sequence of digits sent to the gateway.
     * 
     * @param areaCode single are code for which patterns will be generated
     * @return a single call pattern
     */
    CallPattern calculateCallPattern(String areaCode) {
        CallPattern callPattern = new CallPattern(m_longDistancePrefix + areaCode,
                CallDigits.VARIABLE_DIGITS);
        return callPattern;
    }

    public Transform[] getTransforms() {
        throw new UnsupportedOperationException(
                "getTransforms not implemented for LongDistance rule");
    }

    public void appendToGenerationRules(List<DialingRule> rules) {
        if (!isEnabled()) {
            return;
        }
        String[] areaPatterns = DialPattern.getPatternsFromList(m_areaCodes, StringUtils.EMPTY);
        if (0 == areaPatterns.length) {
            CustomDialingRule rule = createCustomRule(StringUtils.EMPTY);
            rule.setDescription(getDescription());
            rules.add(rule);
        } else {
            for (int i = 0; i < areaPatterns.length; i++) {
                String areaCode = areaPatterns[i];
                CustomDialingRule rule = createCustomRule(areaCode);
                rule.setDescription(getDescription());
                rules.add(rule);
            }
        }
    }

    /**
     * External rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return false;
    }

    /**
     * Creates a single custom rule that will be used to generate dial and call patterns for a
     * specified areaCode
     * 
     * @param areaCode area code inserted in dial and call patterns
     * @return newly created custom rule
     */
    private CustomDialingRule createCustomRule(String areaCode) {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setName(getName());
        rule.setDescription(getDescription());
        rule.setEnabled(isEnabled());
        rule.setGateways(getGateways());
        rule.setCallPattern(calculateCallPattern(areaCode));
        rule.setDialPatterns(calculateDialPatterns(areaCode));
        List perms = Collections.singletonList(getPermission());
        rule.setPermissions(perms);
        return rule;
    }

    public DialingRuleType getType() {
        return DialingRuleType.LONG_DISTANCE;
    }

    public String getAreaCodes() {
        return m_areaCodes;
    }

    public void setAreaCodes(String areaCodes) {
        m_areaCodes = areaCodes;
    }

    public int getExternalLen() {
        return m_externalLen;
    }

    public void setExternalLen(int externalLen) {
        m_externalLen = externalLen;
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

    public Permission getPermission() {
        return m_permission;
    }

    public void setPermission(Permission permission) {
        m_permission = permission;
    }

    public void setPermissionName(String permisisonName) {
        setPermission(getPermission(permisisonName));
    }

    public boolean isPstnPrefixOptional() {
        return m_pstnPrefixOptional;
    }

    public void setPstnPrefixOptional(boolean pstnPrefixOptional) {
        m_pstnPrefixOptional = pstnPrefixOptional;
    }

    public boolean isLongDistancePrefixOptional() {
        return m_longDistancePrefixOptional;
    }

    public void setLongDistancePrefixOptional(boolean longDistancePrefixOptional) {
        m_longDistancePrefixOptional = longDistancePrefixOptional;
    }
}
