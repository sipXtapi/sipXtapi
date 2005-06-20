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
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.gateway.Gateway;

/**
 * EmergencyRouting
 */
public class EmergencyRouting {
    private Gateway m_defaultGateway;
    private String m_externalNumber;

    private Collection m_exceptions = new ArrayList();

    public void addException(RoutingException exception) {
        m_exceptions.add(exception);
    }

    public void removeException(RoutingException exception) {
        m_exceptions.remove(exception);
    }

    public void removeException(Integer exceptionId) {
        m_exceptions.remove(new BeanWithId(exceptionId));
    }

    // getters and setters
    public Gateway getDefaultGateway() {
        return m_defaultGateway;
    }

    public void setDefaultGateway(Gateway defaultGateway) {
        m_defaultGateway = defaultGateway;
    }

    public Collection getExceptions() {
        return m_exceptions;
    }

    public void setExceptions(Collection exceptions) {
        m_exceptions = exceptions;
    }

    public String getExternalNumber() {
        return m_externalNumber;
    }

    public void setExternalNumber(String externalNumber) {
        m_externalNumber = externalNumber;
    }

    /**
     * Converts emergency dialing object to the list of dialing rules. It is used to generate
     * proper authorization rules.
     * 
     * @return list of DialingRules
     */
    public List asDialingRulesList() {
        ArrayList rules = new ArrayList();
        if (null != m_defaultGateway && StringUtils.isNotBlank(m_externalNumber)) {
            DialingRule rule = createDialRule(m_defaultGateway, m_externalNumber);
            rules.add(rule);
        }
        for (Iterator i = m_exceptions.iterator(); i.hasNext();) {
            RoutingException re = (RoutingException) i.next();
            DialingRule rule = createDialRule(re.getGateway(), re.getExternalNumber());
            rules.add(rule);
        }
        return rules;
    }

    /**
     * Creates custom dial rule: one gateway, fixed pattern, no translation, no permission
     * 
     * @param gateway rule gateway
     * @param externalNumber rule dial patter
     * @return a newly create dial rule
     */
    private DialingRule createDialRule(Gateway gateway, String externalNumber) {
        CustomDialingRule rule = new CustomDialingRule();
        rule.setName("caller sensitive emergency routing");
        rule.setGateways(Collections.singletonList(gateway));
        rule.setDialPatterns(Collections.singletonList(new DialPattern(externalNumber, 0)));
        return rule;
    }
}
