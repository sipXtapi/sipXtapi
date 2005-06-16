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
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;

public class AliasServiceImplTest extends XMLTestCase {
    private final static String[][] FORWARDING_DATA = {
        {
            "301@example.org", "\"John Doe\"<sip:john.doe@example.org>"
        }, {
            "302@example.org", "\"Jane Doe\"<sip:jane.doe@example.org>;q=0.5"
        }, {
            "302@example.org", "\"Betty Boop\"<sip:betty.boop@example.org>;q=0.8"
        }
    };

    private final static String[][] CALLGROUP_DATA = {
        {
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
        assertFalse(XMLUnit.getControlParser().isNamespaceAware());

        List forwardingAliases = createAliasMappings(FORWARDING_DATA);

        MockControl mockForwardingContext = MockControl
                .createStrictControl(ForwardingContext.class);
        ForwardingContext forwardingContext = (ForwardingContext) mockForwardingContext.getMock();
        forwardingContext.getForwardingAliases();
        mockForwardingContext.setReturnValue(forwardingAliases);
        mockForwardingContext.replay();

        List callGroupAliases = createAliasMappings(CALLGROUP_DATA);

        MockControl mockCallGroupContext = MockControl
                .createStrictControl(CallGroupContext.class);
        CallGroupContext callGroupContext = (CallGroupContext) mockCallGroupContext.getMock();
        callGroupContext.getAliases();
        mockCallGroupContext.setReturnValue(callGroupAliases);
        mockCallGroupContext.replay();

        AliasServiceImpl impl = new AliasServiceImpl();
        impl.setForwardingContext(forwardingContext);
        impl.setCallGroupContext(callGroupContext);
        String aliases = impl.getForwardingAliases();

        InputStream referenceXmlStream = AliasServiceImplTest.class
                .getResourceAsStream("alias.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(aliases));

        mockForwardingContext.verify();
        mockCallGroupContext.verify();
    }

    /**
     * Builds list of Alias mappings from tabular data
     * 
     * @return liss of AliasMapping objects
     */
    private List createAliasMappings(String[][] data) {
        List aliases = new ArrayList();
        for (int i = 0; i < data.length; i++) {
            String[] aliasRow = data[i];
            AliasMapping mapping = new AliasMapping();
            mapping.setIdentity(aliasRow[0]);
            mapping.setContact(aliasRow[1]);
            aliases.add(mapping);
        }
        return aliases;
    }
}
