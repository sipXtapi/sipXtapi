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
package org.sipfoundry.sipxconfig.conference;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;

public class ConferenceBridgeContextImplTestDb extends SipxDatabaseTestCase {

    private ConferenceBridgeContext m_context;

    protected void setUp() throws Exception {
        m_context = (ConferenceBridgeContext) TestHelper.getApplicationContext().getBean(
                ConferenceBridgeContext.CONTEXT_BEAN_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("conference/users.db.xml");
    }

    public void testDeploy() throws Exception {
        MockControl dbCtrl = MockControl.createControl(ConfigDbParameter.class);
        ConfigDbParameter db = (ConfigDbParameter) dbCtrl.getMock();
        db.set("bbridge-conf", null);
        // do not check params for now - we just verify that the function has been called once
        dbCtrl.setMatcher(MockControl.ALWAYS_MATCHER);
        dbCtrl.setReturnValue(10);

        dbCtrl.replay();

        ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
        adaptor.setConfigDbParameter(db);

        TestHelper.insertFlat("conference/participants.db.xml");
        Bridge bridge = m_context.loadBridge(new Integer(2006));

        ConferenceBridgeProvisioningImpl impl = new ConferenceBridgeProvisioningImpl();
        impl.deploy(bridge, adaptor);

        dbCtrl.verify();
    }
}
