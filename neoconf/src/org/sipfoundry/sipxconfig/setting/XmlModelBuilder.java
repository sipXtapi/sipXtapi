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
import java.util.ArrayList;

import org.apache.commons.digester.Digester;
import org.xml.sax.SAXException;

/**
 * Build a model schema from an XML file
 */
public class XmlModelBuilder {
    
    public SettingModel buildModel(InputStream is) throws IOException {
        Digester digester = new Digester();
        digester.setValidating(false);
        String metaSetter = "addMeta";
        String modelPattern = "*/model";
        digester.addObjectCreate(modelPattern, SettingModel.class);
        digester.addSetNext(modelPattern, metaSetter, SettingModel.class.getName());
        digester.addSetProperties(modelPattern);

        String metaPattern = "*/model/meta";
        digester.addSetNext(metaPattern, metaSetter, SettingMeta.class.getName());
        digester.addObjectCreate(metaPattern, SettingMeta.class);
        digester.addSetProperties(metaPattern);

        digester.addObjectCreate("*/meta/enum", ArrayList.class);
        digester.addObjectCreate("*/enum/element", String.class);
        
        try {
            return (SettingModel) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }
}
