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

import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * ConfigGenerator
 */
public class ConfigGenerator {
    // TODO: change to new to generate files directly
    private static final String SUFFIX = ".in.new";

    private MappingRules m_mapping;
    private AuthRules m_auth;
    private FallbackRules m_fallback;
    private Map m_files = new HashMap();

    public ConfigGenerator() {
        m_mapping = new MappingRules();
        m_auth = new AuthRules();
        m_fallback = new FallbackRules();
        m_files.put(ConfigFileType.MAPPING_RULES, m_mapping);
        m_files.put(ConfigFileType.FALLBACK_RULES, m_fallback);
        m_files.put(ConfigFileType.AUTH_RULES, m_auth);
    }

    public void generate(FlexibleDialPlanContext plan) {
        List rules = plan.getGenerationRules();

        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_mapping.generate(rule);
            m_auth.generate(rule);
            m_fallback.generate(rule);
        }
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

    public void activate(String directory) throws IOException {
        List types = ConfigFileType.getEnumList();
        File configDir = new File(directory);
        for (Iterator i = types.iterator(); i.hasNext();) {
            ConfigFileType type = (ConfigFileType) i.next();
            String filename = type.getName() + SUFFIX;
            XmlFile file = (XmlFile) m_files.get(type);
            file.writeToFile(configDir, filename);
        }
    }
}
