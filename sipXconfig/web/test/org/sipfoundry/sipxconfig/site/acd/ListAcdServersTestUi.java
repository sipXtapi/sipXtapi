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
package org.sipfoundry.sipxconfig.site.acd;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.ListWebTestCase;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ListAcdServersTestUi extends ListWebTestCase {
    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ListAcdServersTestUi.class);
    }

    public ListAcdServersTestUi() {
        super("listAcdServers", "resetAcdContext", "server");
        setHasDuplicate(false);
        setExactCheck(true);
    }

    protected String[] getParamNames() {
        return new String[] {
            "hostField", "portField"
        };
    }

    protected String[] getParamValues(int i) {
        return new String[] {
            "localhost" + i, Integer.toString(8100 + i)
        };
    }

    public void testDisplayEdit() throws Exception {
        clickAddLink();
        SiteTestHelper.assertNoException(tester);
        assertButtonPresent("form:ok");
        assertButtonPresent("form:cancel");
    }

    public void testPresenceServerLink() throws Exception {
        clickLink("link:presence");
        assertElementPresent("setting:SIP_PRESENCE_SIGN_IN_CODE");
    }
}
