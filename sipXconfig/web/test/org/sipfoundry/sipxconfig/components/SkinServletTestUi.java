/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.InputStream;
import java.net.URL;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class SkinServletTestUi extends WebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(SkinServletTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
    }
    
    public void testLogoAsset() throws Exception {        
        assertAssetEquals("/skin/logo.png", "test-logo.png");
        assertAssetEquals("/skin/unused.png", "unused.png");
    }
    
    private void assertAssetEquals(String assetPath, String expectedAsset) throws Exception {
        URL logoUrl = new URL(SiteTestHelper.getBaseUrl() + assetPath);
        InputStream actual = logoUrl.openStream();
        InputStream expected = getClass().getResourceAsStream(expectedAsset);
        assertTrue(IOUtils.contentEquals(expected, actual));
    }
}
