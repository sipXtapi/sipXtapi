
import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import org.sipfoundry.sipxconfig.site.JettyTestSetup;
import org.sipfoundry.sipxconfig.site.line.EditLineTestUi;
import org.sipfoundry.sipxconfig.site.line.LineSettingsTestUi;
import org.sipfoundry.sipxconfig.site.phone.AddPhoneUserTestUi;
import org.sipfoundry.sipxconfig.site.phone.EditPhoneLinesTestUi;
import org.sipfoundry.sipxconfig.site.phone.EditPhoneSettingsTestUi;
import org.sipfoundry.sipxconfig.site.phone.EditPhoneTestUi;
import org.sipfoundry.sipxconfig.site.phone.ManagePhonesTestUi;
import org.sipfoundry.sipxconfig.site.phone.NewPhoneTestUi;
import org.sipfoundry.sipxconfig.site.ui.DialPlanEditTestUi;
import org.sipfoundry.sipxconfig.site.ui.GatewaysTestUi;

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

/**
 * Share one webserver with all ui unittests.  Unfortunately I do not know how to recursively
 * include all classes that match *TestUi from JUnit w/o messy file filtering.
 */
public class UiTestSuite extends TestCase {
    
    public static Test suite() throws Exception {
        TestSuite suite = new TestSuite();        
        suite.addTest(DialPlanEditTestUi.suite());
        suite.addTest(GatewaysTestUi.suite());
        suite.addTest(LineSettingsTestUi.suite());
        suite.addTest(EditLineTestUi.suite());
        suite.addTest(EditPhoneLinesTestUi.suite());
        suite.addTest(NewPhoneTestUi.suite());
        suite.addTest(ManagePhonesTestUi.suite());
        suite.addTest(EditPhoneSettingsTestUi.suite());
        suite.addTest(AddPhoneUserTestUi.suite());
        suite.addTest(EditPhoneTestUi.suite());
        JettyTestSetup jetty = new JettyTestSetup(suite);        
        return jetty;        
    }
}
