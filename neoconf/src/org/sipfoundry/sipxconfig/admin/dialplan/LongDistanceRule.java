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
import org.sipfoundry.sipxconfig.admin.dialplan.config.Permission;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;

/**
 * LongDistanceRule
 */
public class LongDistanceRule extends DialingRule {
    private String m_pstnPrefix = StringUtils.EMPTY;
    private String m_longDistancePrefix = StringUtils.EMPTY;
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
    List calculateDialPatterns(String areaCode) {
        int variableLenght = m_externalLen - areaCode.length();
        ArrayList patterns = new ArrayList();
        if (StringUtils.isNotBlank(m_pstnPrefix)) {
            DialPattern patternFull = new DialPattern(m_pstnPrefix + m_longDistancePrefix
                    + areaCode, variableLenght);
            patterns.add(patternFull);
        }
        if (StringUtils.isNotBlank(m_longDistancePrefix)) {
            DialPattern patternNormal = new DialPattern(m_longDistancePrefix + areaCode,
                    variableLenght);
            patterns.add(patternNormal);
        }
        DialPattern patternShort = new DialPattern(areaCode, variableLenght);
        patterns.add(patternShort);
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

    public void appendToGenerationRules(List rules) {
        if (!isEnabled()) {
            return;
        }
        String[] areaPatterns = DialPattern.getPatternsFromList(m_areaCodes, StringUtils.EMPTY);
        if (0 == areaPatterns.length) {
            CustomDialingRule rule = createCustomRule(StringUtils.EMPTY);
            rules.add(rule);
        } else {
            for (int i = 0; i < areaPatterns.length; i++) {
                String areaCode = areaPatterns[i];
                CustomDialingRule rule = createCustomRule(areaCode);
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
}
