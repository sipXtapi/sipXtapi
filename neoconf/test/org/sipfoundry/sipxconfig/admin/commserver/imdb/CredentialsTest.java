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

import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class CredentialsTest extends XMLTestCase {
    public void testGenerateEmpty() throws Exception {
        MockControl control = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) control.getMock();
        coreContext.getDomainName();
        control.setReturnValue("host.company.com");
        coreContext.getAuthorizationRealm();
        control.setReturnValue("company.com");
        coreContext.loadUsers();
        control.setReturnValue(Collections.EMPTY_LIST);
        control.replay();

        Credentials credentials = new Credentials();
        credentials.setCoreContext(coreContext);

        Document document = credentials.generate();
        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("credential", "/items/@type", domDoc);
        assertXpathNotExists("/items/item", domDoc);
        control.verify();
    }

    public void testAddUser() throws Exception {
        Document document = DocumentFactory.getInstance().createDocument();
        Element item = document.addElement("items").addElement("item");
        
        User user = new User();
        user.setDisplayId("superadmin");
        user.setPintoken("pin1234");
        user.setSipPassword("pass4321");
        
        Credentials credentials = new Credentials();
        credentials.addUser(item,user,"sipx.sipfoundry.org","sipfoundry.org");

        org.w3c.dom.Document domDoc = XmlUnitHelper.getDomDoc(document);
        assertXpathEvaluatesTo("sip:superadmin@sipx.sipfoundry.org", "/items/item/uri", domDoc);
        assertXpathEvaluatesTo("pin1234", "/items/item/pintoken", domDoc);
        assertXpathEvaluatesTo("pass4321", "/items/item/passtoken", domDoc);
        assertXpathEvaluatesTo("sipfoundry.org", "/items/item/realm", domDoc);
        assertXpathEvaluatesTo("DIGEST", "/items/item/authtype", domDoc);
    }
}
