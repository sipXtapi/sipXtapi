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

import java.io.IOException;
import java.io.InputStream;

import org.apache.commons.digester.Digester;
import org.xml.sax.SAXException;

/**
 * Build a model schema from an XML file
 */
public class XmlModelBuilder {
    
    public SettingModel buildModel(InputStream is) throws IOException {
        Digester digester = new Digester();
        digester.setValidating(false);
        String addMeta = "addMeta";

        digester.addObjectCreate("set", SettingModel.class);

        String set = "*/set";
        digester.addObjectCreate(set, SettingModel.class);
        digester.addSetProperties(set);
        digester.addBeanPropertySetter(set + "/label");
        digester.addSetNext(set, addMeta, SettingMeta.class.getName());

        String setting = "*/setting";
        digester.addObjectCreate(setting, SettingMeta.class);
        digester.addSetProperties(setting);
        digester.addSetNext(setting, addMeta, SettingMeta.class.getName());

        String possibleValues = "*/possibleValues/value";
        digester.addCallMethod(possibleValues, "addPossibleValue", 1);
        digester.addCallParam(possibleValues, 0);
        
        try {
            return (SettingModel) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }
}
