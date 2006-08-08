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

import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class DialingRuleFactory implements BeanFactoryAware {
    private Collection<String> m_beanIds;

    private BeanFactory m_beanFactory;

    /**
     * Constructs dialing rule from prototypes defined in Spring configuration file.
     * 
     * Throws illegal argument exception if invalid or unregistered type is passed.
     * 
     * @param type dialing rule type
     * @return newly created object
     */
    public DialingRule create(String beanId) {
        DialingRule rule = (DialingRule) m_beanFactory.getBean(beanId, DialingRule.class);
        // reset new rule - we do not want to suggest invalid values for name, description etc.
        rule.setEnabled(false);
        rule.setDescription(StringUtils.EMPTY);
        rule.setName(StringUtils.EMPTY);
        return rule;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public void setBeanIds(Collection<String> beanIds) {
        m_beanIds = beanIds;
    }

    public Collection<String> getBeanIds() {
        return m_beanIds;
    }
}
