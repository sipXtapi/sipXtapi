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

import junit.framework.TestCase;


public class ModelBuilderTest extends TestCase {
    
    public void testBuilder() throws IOException {
        XmlModelBuilder builder = new XmlModelBuilder();
        InputStream in = getClass().getResourceAsStream("simplemodel.xml");
        SettingModel root = builder.buildModel(in);
        assertEquals(1, root.size());
        SettingModel first = (SettingModel) root.values().iterator().next();        
        assertEquals(4, first.size());
    }
}
