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
import java.io.IOException;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;


public class PatternSettingFilterTest extends TestCase {
    
    public void testPatternFilter() throws IOException {
        ModelBuilder builder = new XmlModelBuilder("etc");
        File in = TestHelper.getResourceAsFile(getClass(), "games.xml");        
        SettingSet games = builder.buildModel(in);

        PatternSettingFilter filter = new PatternSettingFilter();
        filter.addExcludes("cards.*$");
        filter.addExcludes("chess/piece.*$");
        Collection settings = SettingUtil.filter(filter, games);
        assertEquals(4, settings.size());        
    }
}
