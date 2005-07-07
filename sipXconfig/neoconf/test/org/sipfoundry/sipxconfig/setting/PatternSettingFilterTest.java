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
import java.util.Collection;

import junit.framework.TestCase;


public class PatternSettingFilterTest extends TestCase {
    
    public void testPatternFilter() throws IOException {
        XmlModelBuilder builder = new XmlModelBuilder("etc");
        InputStream in = getClass().getResourceAsStream("games.xml");
        SettingSet games = builder.buildModel(in);

        PatternSettingFilter filter = new PatternSettingFilter();
        filter.addExcludes("/cards.*$");
        filter.addExcludes("/chess/piece.*$");
        Collection settings = FilterRunner.filter(filter, games);
        assertEquals(4, settings.size());        
    }
}
