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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Config file based storage implementation. Setting.profileName property is used to identify
 * configuration file Files are read using standard java Properties and written by
 * ConfigFileStorage
 * 
 * @see ConfigFileStorage
 */
public class ConfigFileStorage extends ValueStorage {
    private Map m_file2properties = new HashMap();

    private String m_configDirectory;

    public ConfigFileStorage(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public Object getValue(Setting setting) {
        try {
            Properties properties = loadForFile(setting);
            return properties.getProperty(setting.getName());
        } catch (IOException e) {
            return StringUtils.EMPTY;
        }
    }

    public Object setValue(Setting setting, Object value) {
        try {
            Properties properties = loadForFile(setting);
            return properties.put(setting.getName(), value);
        } catch (IOException e) {
            return StringUtils.EMPTY;
        }
    }
    
    String nonNull(String value) {
        return (value == null ? StringUtils.EMPTY : value);
    }

    /**
     * Remove is called when setting is set to default value.
     */
    public Object remove(Setting setting) {
        try {
            Properties properties = loadForFile(setting);
            return properties.put(setting.getName(), nonNull(setting.getValue()));
        } catch (IOException e) {
            return StringUtils.EMPTY;
        }
    }

    private Properties loadForFile(Setting setting) throws IOException {
        String filename = setting.getProfileName();
        Properties properties = (Properties) m_file2properties.get(filename);
        if (null != properties) {
            return properties;
        }
        File file = new File(m_configDirectory, filename);
        FileInputStream stream = new FileInputStream(file);
        properties = new Properties();
        properties.load(stream);
        IOUtils.closeQuietly(stream);
        m_file2properties.put(filename, properties);
        return properties;
    }

    public void flush() throws IOException {
        for (Iterator i = m_file2properties.entrySet().iterator(); i.hasNext();) {
            Map.Entry entry = (Map.Entry) i.next();
            String fileName = (String) entry.getKey();
            Properties props = (Properties) entry.getValue();
            File file = new File(m_configDirectory, fileName);
            ConfigFileWriter writer = new ConfigFileWriter(file);
            writer.store(props);
        }
    }
}
