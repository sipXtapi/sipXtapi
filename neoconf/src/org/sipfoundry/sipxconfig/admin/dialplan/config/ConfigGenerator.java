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
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.Writer;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.dom4j.Document;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;

import org.sipfoundry.sipxconfig.admin.dialplan.FlexibleDialPlan;
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

    public void generate(FlexibleDialPlan plan) {
        List rules = plan.getGenerationRules();

        for (Iterator i = rules.iterator(); i.hasNext();) {
            IDialingRule rule = (IDialingRule) i.next();
            m_mapping.generate(rule);
            m_auth.generate(rule);
            m_fallback.generate(rule);
        }
    }

    /**
     * Writes the content of the file using supplied writer.
     * 
     * @param type type of the configuration file
     * @param writer standard Java IO writer (user StringWriter to write to
     *        string)
     * @throws IOException
     */
    public void write(ConfigFileType type, Writer writer) throws IOException {
        ConfigFile file = (ConfigFile) m_files.get(type);
        Document document = file.getDocument();
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        XMLWriter xmlWriter = new XMLWriter(writer, format);
        xmlWriter.write(document);
    }

    /**
     * Retrieves configuration file content as stream.
     * 
     * Use only for preview, use write function to dump it to the file.
     * 
     * @param type type of the configuration file
     */
    public String getFileContent(ConfigFileType type) {
        try {
            StringWriter writer = new StringWriter();
            write(type, writer);
            return writer.toString();
        } catch (IOException e) {
            // ignore when writing to string
            // TODO: log
            return "";
        }
    }

    public void activate(String directory) throws IOException {
        List types = ConfigFileType.getEnumList();
        File configDir = new File(directory);
        for (Iterator i = types.iterator(); i.hasNext();) {
            ConfigFileType type = (ConfigFileType) i.next();
            File configFile = new File(configDir, type.getName() + SUFFIX);
            FileWriter writer = new FileWriter(configFile);
            write(type, writer);
            writer.close();
        }
    }
}
