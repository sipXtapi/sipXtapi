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
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;
import org.xml.sax.Attributes;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * Build a SettingModel object hierarchy from a model XML file.
 */
public class XmlModelBuilder {
    private static final String EL_VALUE = "/value";

    private final EntityResolver m_entityResolver;

    public XmlModelBuilder(File configDirectory) {
        m_entityResolver = new ModelEntityResolver(configDirectory);
    }

    public XmlModelBuilder(String configDirectory) {
        this(new File(configDirectory));
    }

    public SettingGroup buildModel(File modelFile) {
        FileInputStream is = null;
        try {
            is = new FileInputStream(modelFile);
            return buildModel(is);

        } catch (IOException e) {
            throw new RuntimeException("Cannot parse polycom model definitions file "
                    + modelFile.getPath(), e);
        } finally {
            IOUtils.closeQuietly(is);
        }
    }

    public SettingGroup buildModel(InputStream is) throws IOException {
        Digester digester = new Digester();

        // JBoss uses digester and therefore, preloaded, and therefore it's
        // classloader cannot find classes inside deployment containers
        digester.setClassLoader(this.getClass().getClassLoader());

        digester.setValidating(false);
        digester.setEntityResolver(m_entityResolver);
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
            digester.addBeanPropertySetter(m_pattern + EL_VALUE);
            digester.addBeanPropertySetter(m_pattern + "/description");
            digester.addBeanPropertySetter(m_pattern + "/profileName");
            digester.addBeanPropertySetter(m_pattern + "/label");

            digester.addRuleSet(new IntegerSettingRule(m_pattern + "/type/integer"));
            digester.addRuleSet(new StringSettingRule(m_pattern + "/type/string"));
            digester.addRuleSet(new EnumSettingRule(m_pattern + "/type/enum"));

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

    static class SettingTypeRule extends RuleSetBase {
        private final String m_pattern;

        public SettingTypeRule(String pattern) {
            m_pattern = pattern;
        }

        public void addRuleInstances(Digester digester) {
            digester.addSetNext(m_pattern, "setType", SettingType.class.getName());
        }

        public String getPattern() {
            return m_pattern;
        }
    }

    static class StringSettingRule extends SettingTypeRule {
        public StringSettingRule(String pattern) {
            super(pattern);
        }

        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), StringSetting.class);
            digester.addSetProperties(getPattern());
            digester.addBeanPropertySetter(getPattern() + "/pattern");
            super.addRuleInstances(digester);
        }
    }

    static class IntegerSettingRule extends SettingTypeRule {
        public IntegerSettingRule(String pattern) {
            super(pattern);
        }

        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), IntegerSetting.class);
            digester.addSetProperties(getPattern());
            super.addRuleInstances(digester);
        }
    }

    static class EnumSettingRule extends SettingTypeRule {
        public EnumSettingRule(String pattern) {
            super(pattern);
        }

        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), EnumSetting.class);
            String valuePattern = getPattern() + EL_VALUE;
            digester.addCallMethod(valuePattern, "addEnum", 2);
            digester.addCallParam(valuePattern, 0);
            digester.addCallParam(valuePattern, 1, "label");
            super.addRuleInstances(digester);
        }
    }

    private static class ModelEntityResolver implements EntityResolver {
        private static final String DTD = "setting.dtd";

        private File m_dtd;

        ModelEntityResolver(File file) {
            m_dtd = new File(file, DTD);
        }

        public InputSource resolveEntity(String publicId_, String systemId_) throws IOException {
            return new InputSource(new FileInputStream(m_dtd));
        }
    }
}
