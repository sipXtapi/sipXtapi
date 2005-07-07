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
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;

public class AliasServiceImplTest extends XMLTestCase {
    private final static String[][] DATA = {
        {
            "301@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }, {
            "302@example.org", "\"Jane Doe\"<sip:jane.doe@example.org>;q=0.5"
        }, {
            "302@example.org", "\"Betty Boop\"<sip:betty.boop@example.org>;q=0.8"
        }, {
            "302@example.org", "\"Bill Boop\"<sip:bill.boop@example.org>;q=0.8"
        }, {
            "303@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }
    };

    public AliasServiceImplTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetForwardingAliases() throws Exception {
        List aliases = new ArrayList();
        for (int i = 0; i < DATA.length; i++) {
            String[] aliasRow = DATA[i];
            AliasMapping mapping = new AliasMapping();
            mapping.setIdentity(aliasRow[0]);
            mapping.setContact(aliasRow[1]);
            aliases.add(mapping);
        }

        MockControl mockContext = MockControl.createStrictControl(ForwardingContext.class);
        ForwardingContext context = (ForwardingContext) mockContext.getMock();
        context.getForwardingAliases();
        mockContext.setReturnValue(aliases);
        mockContext.replay();

        AliasServiceImpl impl = new AliasServiceImpl();
        impl.setContext(context);
        String forwardingAliases = impl.getForwardingAliases();

        InputStream referenceXmlStream = AliasServiceImplTest.class
                .getResourceAsStream("alias.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(
                forwardingAliases));

        mockContext.verify();
    }
}
