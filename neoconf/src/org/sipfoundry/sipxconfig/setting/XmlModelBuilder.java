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

import java.beans.PropertyDescriptor;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.util.Arrays;

import org.apache.commons.beanutils.PropertyUtilsBean;
import org.apache.commons.digester.Digester;
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
        String addSetting = "addSetting";
        digester.push(new SettingGroup());

        digester.addObjectCreate("group", SettingGroup.class);

        String set = "*/group";
        String[] setIgnore = new String[] {
            "setting", "possibleValues" 
        };
        Arrays.sort(setIgnore);
        digester.addObjectCreate(set, SettingGroup.class);
        digester.addSetProperties(set);
        addAllBeanPropertySetter(digester, set, setIgnore, SettingGroup.class);
        digester.addBeanPropertySetter(set + "/label");
        digester.addSetNext(set, addSetting, SettingImpl.class.getName());

        String setting = "*/setting";
        // because setting in baseclass to set, this should be a subset of same ignore
        // list
        String[] settingIgnore = setIgnore;
        digester.addObjectCreate(setting, SettingImpl.class);
        digester.addSetProperties(setting);
        addAllBeanPropertySetter(digester, setting, settingIgnore, SettingImpl.class);
        digester.addSetNext(setting, addSetting, SettingImpl.class.getName());

        String possibleValues = "*/possibleValues/value";
        digester.addCallMethod(possibleValues, "addPossibleValue", 1);
        digester.addCallParam(possibleValues, 0);

        try {
            return (SettingGroup) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }

    /**
     * Get all setters on bean and automatically add them to digester's current rule. This
     * is so you can invent a new element on the model definition (dtd eventually) and 
     * add a new setter on the respective bean and that's it.  You will from time to time
     * have to add to the ignore list, but this seems much less work IMHO.
     * 
     * NOTE: checkstyle was complaining this method was unused if i made it private.  make
     * package protected for now.
     */
    void addAllBeanPropertySetter(Digester digester, String pattern, String[] ignore,
            Class c) {
        PropertyDescriptor[] props = new PropertyUtilsBean().getPropertyDescriptors(c);
        for (int i = 0; i < props.length; i++) {
            Method m = props[i].getWriteMethod();
            String propName = props[i].getName();
            if (m != null && Arrays.binarySearch(ignore, propName) < 0) {
                String propPattern = pattern + "/" + propName;
                digester.addBeanPropertySetter(propPattern);
            }
        }
    }
}
