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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class ExtensionsTest extends XMLTestCase {
    private String[][] DATA = {
        {
            "first", "last", "userName", "1234"
        }, {
            null, null, "kuku", "4321"
        }, {
            "user", "without", "extension", null
        },
    };

    private final List m_users;

    public ExtensionsTest() {
        List users = new ArrayList();
        for (int i = 0; i < DATA.length; i++) {
            String[] userData = DATA[i];
            User user = new User();
            user.setFirstName(userData[0]);
            user.setLastName(userData[1]);
            user.setUserName(userData[2]);
            String extension = userData[3];
            if (!StringUtils.isBlank(extension)) {
                user.getAliases().add(extension);                
            }
            users.add(user);
        }
        m_users = Collections.unmodifiableList(users);
    }

    public void testGenerateEmpty() throws Exception {
        IMocksControl control = EasyMock.createControl();
        CoreContext coreContext = control.createMock(CoreContext.class);
        coreContext.getDomainName();
        control.andReturn("company.com");
        coreContext.loadUsers();
        control.andReturn(Collections.EMPTY_LIST);
        control.replay();

        Extensions extensions = new Extensions();
        extensions.setCoreContext(coreContext);

        Document document = extensions.generate();
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("extension", "/items/@type", domDoc);
        assertXpathNotExists("/items/item", domDoc);
        control.verify();
    }
    
    public void testGenerate() throws Exception {
        IMocksControl control = EasyMock.createControl();
        CoreContext coreContext = control.createMock(CoreContext.class);
        coreContext.getDomainName();
        control.andReturn("company.com");
        coreContext.loadUsers();
        control.andReturn(m_users);
        control.replay();

        Extensions extensions = new Extensions();
        extensions.setCoreContext(coreContext);

        Document document = extensions.generate();
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);

        assertXpathEvaluatesTo("extension", "/items/@type", domDoc);
        assertXpathEvaluatesTo("1234", "/items/item/extension", domDoc);
        assertXpathEvaluatesTo("\"first last\"<sip:userName@company.com>", "/items/item/uri", domDoc);
        assertXpathEvaluatesTo("4321", "/items/item[2]/extension", domDoc);
        assertXpathEvaluatesTo("sip:kuku@company.com", "/items/item[2]/uri", domDoc);
        assertXpathNotExists("/items/item[3]", domDoc);

        control.verify();
    }
}
