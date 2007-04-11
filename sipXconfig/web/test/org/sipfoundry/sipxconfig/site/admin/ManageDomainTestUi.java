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
package org.sipfoundry.sipxconfig.site.admin;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManageDomainTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManageDomainTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(getTester());
        clickLink("link:domain");
    }
    
    public void testDisplay() {
        assertElementPresent("domain:name");
    }
        
    public void testUpdateDomain() {        
        String existingDomain = getDialog().getFormParameterValue("name");
        String newDomain = StringUtils.reverse(existingDomain);
        assertNotSame("test incorrectly assumed domain name is a palindrome", newDomain, existingDomain);
        setFormElement("name", newDomain);
        clickButton("form:apply");
        assertButtonPresent("activate");
        // get NPE if i activate
        clickButton("cancel");
        assertFormElementEquals("name", newDomain);
    }
}