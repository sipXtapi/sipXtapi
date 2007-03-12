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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.io.File;
import java.util.Collection;
import java.util.Iterator;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ServerSettingsTestUi extends WebTestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ServerSettingsTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.home(tester);
        clickLink(ServerSettings.PAGE);
    }

    public void testDisplay() {
        SiteTestHelper.assertNoException(tester);
        SiteTestHelper.assertNoUserError(tester);

        // click on all the links on the page

        String etcDir = SiteTestHelper.getArtificialSystemRootDirectory() + "/etc";
        assertNotNull(etcDir);

        File settingDir = new File(etcDir, "commserver");
        File modelDefsFile = new File(settingDir, "server.xml");
        Setting model = new XmlModelBuilder(etcDir).buildModel(modelDefsFile).copy();
        Collection sections = model.getValues();
        assertFalse(sections.isEmpty());

        for (Iterator i = sections.iterator(); i.hasNext();) {
            Setting section = (Setting) i.next();
            String linkId = "link:" + section.getLabelKey();
            if (section.isHidden()) {
                assertLinkNotPresent(linkId);
            } else {
                clickLink(linkId);
                SiteTestHelper.assertNoException(tester);
                SiteTestHelper.assertNoUserError(tester);
            }
        }
    }
}
