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
package org.sipfoundry.sipxconfig.settings;

import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.SettingModel;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;


public class ModelBuilderTest extends TestCase {
    
    public void testBuilder() throws IOException {
        /* Work in progress
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("simplemodel.xml");
        SettingModel model = builder.buildModel(in);
        assertEquals(4, model.size());
        */
    }
}
