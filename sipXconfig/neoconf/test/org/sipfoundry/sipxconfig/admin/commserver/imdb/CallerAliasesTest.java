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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

public class CallerAliasesTest extends XMLTestCase {
    public CallerAliasesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    private String[][] USER_DATA = {
        {
            "first", "last", "userName", "1234"
        }, {
            null, null, "kuku", "4321"
        }, {
            "user", "without", "number", null
        },
    };

    private String[][] GATEWAY_DATA = {
        {
            "example.org", "7832331111"
        }, {
            "bongo.com", null
        }
    };

    private List<User> m_users;

    private List<Gateway> m_gateways;

    protected void setUp() throws Exception {
        m_users = new ArrayList<User>();
        for (String[] ud : USER_DATA) {
            User user = new User();
            user.setFirstName(ud[0]);
            user.setLastName(ud[1]);
            user.setUserName(ud[2]);
            user.setExternalNumber(ud[3]);
            m_users.add(user);
        }
        
        m_gateways = new ArrayList<Gateway>();
        for (String[] gd : GATEWAY_DATA) {
            Gateway gateway = new Gateway();
            gateway.setAddress(gd[0]);
            gateway.setDefaultCallerAlias(gd[1]);
            m_gateways.add(gateway);
        }
    }

    public void testGenerateEmpty() throws Exception {
        CallerAliases cas = new CallerAliases();
        cas.setSipDomain("example.org");

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        EasyMock.expectLastCall().andReturn(Collections.emptyList());

        GatewayContext gatewayContext = EasyMock.createMock(GatewayContext.class);
        gatewayContext.getGateways();
        EasyMock.expectLastCall().andReturn(Collections.emptyList());

        cas.setCoreContext(coreContext);
        cas.setGatewayContext(gatewayContext);

        EasyMock.replay(coreContext, gatewayContext);

        Document document = cas.generate();
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("caller-alias", "/items/@type", domDoc);
        assertXpathNotExists("/items/item", domDoc);

        EasyMock.verify(coreContext, gatewayContext);
    }

    public void testGenerate() throws Exception {
        CallerAliases cas = new CallerAliases();
        cas.setSipDomain("example.org");

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.loadUsers();
        EasyMock.expectLastCall().andReturn(m_users);

        GatewayContext gatewayContext = EasyMock.createMock(GatewayContext.class);
        gatewayContext.getGateways();
        EasyMock.expectLastCall().andReturn(m_gateways);

        cas.setCoreContext(coreContext);
        cas.setGatewayContext(gatewayContext);

        EasyMock.replay(coreContext, gatewayContext);

        Document document = cas.generate();
        
        String casXml = XmlUnitHelper.asString(document);
        InputStream referenceXmlStream = AliasesTest.class.getResourceAsStream("caller-alias.test.xml");
        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(casXml));
        
        
        EasyMock.verify(coreContext, gatewayContext);
    }
    
}
