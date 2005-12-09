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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    public static final String BEAN_NAME = "dialPlanConfigGenerator";

    private MappingRules m_mappingRules;
    private AuthRules m_authRules;
    private FallbackRules m_fallbackRules;
    private Map m_files = new HashMap();

    public ConfigGenerator() {
        // this is usually overwritten by spring configuration file
        setMappingRules(new MappingRules());
        setAuthRules(new AuthRules());
        setFallbackRules(new FallbackRules());
    }

    public void setAuthRules(AuthRules authRules) {
        m_authRules = authRules;
        m_files.put(ConfigFileType.AUTH_RULES, m_authRules);
    }

    public void setMappingRules(MappingRules mappingRules) {
        m_mappingRules = mappingRules;
        m_files.put(ConfigFileType.MAPPING_RULES, mappingRules);
    }

    public void setFallbackRules(FallbackRules fallbackRules) {
        m_fallbackRules = fallbackRules;
        m_files.put(ConfigFileType.FALLBACK_RULES, m_fallbackRules);
    }

    public void generate(EmergencyRouting er) {
        List rules = er.asDialingRulesList();
        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_authRules.generate(rule);
        }
    }

    public void generate(DialPlanContext plan) {
        List rules = plan.getGenerationRules();
        m_mappingRules.begin();
        m_authRules.begin();
        m_fallbackRules.begin();

        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_mappingRules.generate(rule);
            m_authRules.generate(rule);
            m_fallbackRules.generate(rule);
        }
        m_mappingRules.end();
        m_authRules.end();
        m_fallbackRules.end();
    }

    /**
     * Retrieves configuration file content as stream.
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     * @param type type of the configuration file
     */
    public String getFileContent(ConfigFileType type) {
        XmlFile file = (XmlFile) m_files.get(type);
        return file.getFileContent();
    }

    public void activate(SipxReplicationContext sipxReplicationContext) {
        for (Iterator i = m_files.values().iterator(); i.hasNext();) {
            XmlFile file = (XmlFile) i.next();
            sipxReplicationContext.replicate(file);
        }
    }
}
