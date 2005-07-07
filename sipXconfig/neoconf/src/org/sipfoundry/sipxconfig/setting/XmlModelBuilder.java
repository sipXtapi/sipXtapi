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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.digester.Digester;
import org.apache.commons.digester.Rule;
import org.apache.commons.digester.RuleSetBase;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

/**
 * Build a SettingModel object hierarchy from a model XML file.
 */
public class XmlModelBuilder {
    
    public SettingGroup buildModel(File modelFile) {
        SettingGroup model = null;
        FileInputStream is = null;
        try {
            is = new FileInputStream(modelFile);
            XmlModelBuilder build = new XmlModelBuilder();
            model = build.buildModel(is);

        } catch (IOException e) {
            throw new RuntimeException("Cannot parse polycom model definitions file "
                    + modelFile.getPath(), e);
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException ignore) {
                    // cleanup non fatal
                    ignore.printStackTrace();
                }
            }
        }
        
        return model;        
    }

    public SettingGroup buildModel(InputStream is) throws IOException {
        Digester digester = new Digester();
        
        // JBoss uses digester and therefore, preloaded, and therefore it's
        // classloader cannot find classes inside deployment containers
        digester.setClassLoader(this.getClass().getClassLoader());
        
        digester.setValidating(false);
        digester.push(new SettingGroup());

        String groupPattern = "*/group";
        SettingRuleSet groupRule = new SettingRuleSet(groupPattern, SettingGroup.class);
        digester.addRuleSet(groupRule);

        String settingPattern = "*/setting";
        SettingRuleSet settingRule = new SettingRuleSet(settingPattern, SettingImpl.class);
        digester.addRuleSet(settingRule);

        String possibleValues = "*/possibleValues/value";
        digester.addCallMethod(possibleValues, "addPossibleValue", 1);
        digester.addCallParam(possibleValues, 0);

        try {
            return (SettingGroup) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }
    
    static class SettingRuleSet extends RuleSetBase {
        
        private String m_pattern;
        
        private Class m_class;

        public SettingRuleSet(String pattern, Class c) {
            m_pattern = pattern;
            m_class = c;           
        }

        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(m_pattern, m_class);
            digester.addSetProperties(m_pattern, "parent", null);
            digester.addRule(m_pattern, new CopyOfRule());            
            digester.addBeanPropertySetter(m_pattern + "/value");
            digester.addBeanPropertySetter(m_pattern + "/description");
            digester.addBeanPropertySetter(m_pattern + "/profileName");
            digester.addBeanPropertySetter(m_pattern + "/label");
            digester.addSetNext(m_pattern, "addSetting", SettingImpl.class.getName());
        }
    }
    
    static class CopyOfRule extends Rule {
        
        public void begin(String namespace_, String name_, Attributes attributes) {
            String copyOfName = attributes.getValue("copy-of");
            if (copyOfName != null) {
                Setting copyTo = (Setting) getDigester().pop();
                Setting parent = (Setting) getDigester().peek();
                // setting to be copied must defined in file before setting attempting to copy 
                Setting copyOf = parent.getSetting(copyOfName);
                Setting copy = copyOf.copy();
                copy.setName(copyTo.getName());
                getDigester().push(copy);
            }
        }
    }
}
