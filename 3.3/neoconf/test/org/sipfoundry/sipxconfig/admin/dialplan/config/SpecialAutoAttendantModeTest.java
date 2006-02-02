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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class SpecialAutoAttendantModeTest extends XMLTestCase {

    public SpecialAutoAttendantModeTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDocumentDisabled() throws Exception {
        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode(false, null);
        String expected = "<organizationprefs><specialoperation>false</specialoperation><autoattendant/></organizationprefs>";

        assertXMLEqual(expected, file.getFileContent());
    }

    public void testGetDocument() throws Exception {
        MockControl aaCtrl = MockClassControl.createControl(AutoAttendant.class);
        AutoAttendant aa = (AutoAttendant) aaCtrl.getMock();
        aaCtrl.expectAndReturn(aa.getSystemName(), "afterhours");
        aaCtrl.replay();

        InputStream referenceXmlStream = getClass().getResourceAsStream(
                "organizationprefs.test.xml");

        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode(true, aa);
        String generatedXml = file.getFileContent();
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        aaCtrl.verify();
    }
}
