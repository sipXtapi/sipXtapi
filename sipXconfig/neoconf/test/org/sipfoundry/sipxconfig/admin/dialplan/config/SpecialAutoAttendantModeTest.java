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
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;

public class SpecialAutoAttendantModeTest extends XMLTestCase {

    public SpecialAutoAttendantModeTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDocumentDisabled() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setSystemId("abc");

        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode(false, aa);
        String expected = "<organizationprefs><specialoperation>false</specialoperation><autoattendant>abc</autoattendant></organizationprefs>";

        assertXMLEqual(expected, file.getFileContent());
    }

    public void testGetDocument() throws Exception {
        AutoAttendant aa = new AutoAttendant();
        aa.setSystemId("afterhours");

        InputStream referenceXmlStream = getClass().getResourceAsStream(
                "organizationprefs.test.xml");

        SpecialAutoAttendantMode file = new SpecialAutoAttendantMode(true, aa);
        String generatedXml = file.getFileContent();
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
    }
}
