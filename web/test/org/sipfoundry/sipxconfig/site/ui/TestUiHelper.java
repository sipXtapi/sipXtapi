/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.ui;

import java.io.IOException;
import java.net.MalformedURLException;

import org.xml.sax.SAXException;

import com.meterware.httpunit.GetMethodWebRequest;
import com.meterware.httpunit.WebConversation;
import com.meterware.httpunit.WebLink;
import com.meterware.httpunit.WebRequest;
import com.meterware.httpunit.WebResponse;

/**
 * Several helper methods for writing UI tests
 */
class TestUiHelper {

    public static WebResponse resetDialPlans(WebResponse homePage) throws IOException,
            SAXException {
        final WebLink resetLink = homePage.getLinkWithID("resetDialPlans");
        return resetLink.click();
    }

    /**
     * @return sipxconfig application home page
     */
    public static WebResponse getHomePage() throws MalformedURLException, IOException,
            SAXException {
        WebConversation wc = new WebConversation();
        WebRequest req = new GetMethodWebRequest("http://localhost:8080/sipxconfig");
        WebResponse home = wc.getResponse(req);
        return home;
    }

}
