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

        digester.addObjectCreate("model", SettingModel.class);

        String modelPattern = "*/model";
        digester.addObjectCreate(modelPattern, SettingModel.class);
        digester.addSetNext(modelPattern, addMeta, SettingMeta.class.getName());
        digester.addSetProperties(modelPattern);

        String metaPattern = "*/meta";
        digester.addObjectCreate(metaPattern, SettingMeta.class);
        digester.addSetNext(metaPattern, addMeta, SettingMeta.class.getName());
        digester.addSetProperties(metaPattern);

//        digester.addObjectCreate("*/enum", ArrayList.class);
//        digester.addSetNext(metaPattern, "setPossibleValues", List.class.getName());
//        digester.addObjectCreate("*/element", String.class);
//        digester.addSetNext(metaPattern, "add", List.class.getName());

        try {
            return (SettingModel) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }
}
