/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.user;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.easymock.MockControl;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.DOMOutputter;

import com.pingtel.pds.pgs.organization.OrganizationBusiness;
import com.pingtel.pds.pgs.profile.RefPropertyBusiness;

public class UserHelperTest extends XMLTestCase {
    public UserHelperTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testCalculatePrimaryLineUrl() throws RemoteException {
        MockControl orgControl = MockControl.createControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(), "mycomp.com", 3);
        orgControl.replay();

        MockControl userControl = MockControl.createControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        userControl.expectAndReturn(user.getFirstName(), "First");
        userControl.expectAndReturn(user.getFirstName(), "", 2);
        userControl.expectAndReturn(user.getLastName(), "Last", 2);
        userControl.expectAndReturn(user.getLastName(), "");
        userControl.expectAndReturn(user.getDisplayID(), "displayId", 3);
        userControl.replay();

        UserHelper helper = new UserHelper(user);
        String url = helper.calculatePrimaryLineUrl(org);
        assertEquals("First Last<sip:displayId@mycomp.com>", url);

        url = helper.calculatePrimaryLineUrl(org);
        assertEquals("Last<sip:displayId@mycomp.com>", url);

        url = helper.calculatePrimaryLineUrl(org);
        assertEquals("sip:displayId@mycomp.com", url);

        userControl.verify();
        orgControl.verify();
    }

    public void testCreateInitialLine() throws Exception {
        String password = "password";

        MockControl orgControl = MockControl.createControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(), "mycomp.com", 1);
        orgControl.expectAndReturn(org.getAuthenticationRealm(), "auth.mycomp.com", 2);
        orgControl.replay();

        MockControl userControl = MockControl.createControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        userControl.expectAndReturn(user.getFirstName(), "First", 2);
        userControl.expectAndReturn(user.getLastName(), "Last", 2);
        userControl.expectAndReturn(user.getDisplayID(), "first_last", 4);
        userControl.replay();

        MockControl rpControl = MockControl.createControl(RefPropertyBusiness.class);
        RefPropertyBusiness rp = (RefPropertyBusiness) rpControl.getMock();
        rpControl.expectAndReturn(rp.getID(), new Integer(5));
        rpControl.expectAndReturn(rp.getID(), new Integer(66));
        rpControl.expectAndReturn(rp.getID(), new Integer(3));
        rpControl.replay();

        UserHelper helper = new UserHelper(user);
        String generatedLineXml = helper.createInitialLine(org, rp, rp, rp, password);

        Reader controlXml = new InputStreamReader(getClass().getResourceAsStream(
                "line_profile.test.xml"));

        Diff diff = new Diff(controlXml, new StringReader(generatedLineXml));
        assertXMLEqual(diff, true);

        userControl.verify();
        rpControl.verify();
        orgControl.verify();
    }

    public void testGetDigest() throws RemoteException {
        MockControl orgControl = MockControl.createStrictControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getAuthenticationRealm(), "doto.pingtel.com", 2);
        orgControl.replay();

        MockControl userControl = MockControl.createControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        userControl.expectAndReturn(user.getDisplayID(), "mainuser");
        userControl.expectAndReturn(user.getDisplayID(), "something else");
        userControl.replay();

        UserHelper helper = new UserHelper(user);

        String digest = helper.digestPassword(org, "222");
        assertEquals("66686b8904646f26975a72a486ee9a7e", digest);

        digest = helper.digestPassword(org, "222");
        assertFalse(digest.equals("66686b8904646f26975a72a486ee9a7e"));

        userControl.verify();
        orgControl.verify();
    }

    public void testDigestPassword() throws RemoteException {
        MockControl orgControl = MockControl.createControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(), "doto.pingtel.com", 3);
        orgControl.expectAndReturn(org.getAuthenticationRealm(), "doto.pingtel.com", 1);
        orgControl.expectAndReturn(org.getAuthenticationRealm(), "auth.pingtel.com", 2);
        orgControl.replay();

        MockControl userControl = MockControl.createControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        userControl.expectAndReturn(user.getDisplayID(), "mainuser", 2);
        userControl.expectAndReturn(user.getDisplayID(), "something else");
        userControl.replay();

        UserHelper helper = new UserHelper(user);

        // DNS Domain == authentication realm
        String digest = helper.digestPasswordQualifiedUsername(org, "222");
        assertEquals("9987a2a2d567d33fda1214213a997fb8", digest);

        // DNS Domain != authentication realm
        digest = helper.digestPasswordQualifiedUsername(org, "222");
        assertEquals("d2a6a26a0d092ae2099e09827f28c088", digest);

        digest = helper.digestPasswordQualifiedUsername(org, "222");
        assertFalse(digest.equals("d2a6a26a0d092ae2099e09827f28c088"));

        userControl.verify();
        orgControl.verify();
    }

    public void testFixPrimaryLineCredentials() throws Exception {
        MockControl orgControl = MockControl.createControl(OrganizationBusiness.class);
        OrganizationBusiness org = (OrganizationBusiness) orgControl.getMock();
        orgControl.expectAndReturn(org.getDNSDomain(), "bongo.com");
        orgControl.expectAndReturn(org.getDNSDomain(), "ognob.com");
        orgControl.replay();

        MockControl userControl = MockControl.createControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        userControl.expectAndReturn(user.getFirstName(), "First", 1);
        userControl.expectAndReturn(user.getLastName(), "Last", 1);
        userControl.expectAndReturn(user.getDisplayID(), "first_last", 1);
        userControl.expectAndReturn(user.getFirstName(), "ABC", 1);
        userControl.expectAndReturn(user.getLastName(), "DEF", 1);
        userControl.expectAndReturn(user.getDisplayID(), "abc_def", 2);
        userControl.replay();

        UserHelper helper = new UserHelper(user);

        InputStream stream = getClass().getResourceAsStream("line_profile.test.xml");
        SAXBuilder builder = new SAXBuilder();
        Document document = builder.build(stream);
        Element profile = document.getRootElement();
        Element elementPrimaryLine = profile.getChild("PRIMARY_LINE");

        // replace DNS name - do not touch passtoken
        helper.fixUserPrimaryLine(elementPrimaryLine, org, "auth.bongo.com", false, null);
        DOMOutputter converter = new DOMOutputter();
        org.w3c.dom.Document domDocument = converter.output(document);
        assertXpathEvaluatesTo("First Last<sip:first_last@bongo.com>",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/URL", domDocument);
        assertXpathEvaluatesTo("auth.mycomp.com",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/REALM", domDocument);
        assertXpathEvaluatesTo("first_last",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/USERID", domDocument);
        assertXpathEvaluatesTo("8f44d1d713b8aa4e56c6fd78e8ef1b1a",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/PASSTOKEN", domDocument);

        // this time replace a passtoken, user id and DNS name
        helper.fixUserPrimaryLine(elementPrimaryLine, org, "auth.mycomp.com", true, "kuku");
        domDocument = converter.output(document);
        assertXpathEvaluatesTo("ABC DEF<sip:abc_def@ognob.com>",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/URL", domDocument);
        assertXpathEvaluatesTo("auth.mycomp.com",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/REALM", domDocument);
        assertXpathEvaluatesTo("kuku", "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/PASSTOKEN",
                domDocument);
        assertXpathEvaluatesTo("abc_def",
                "/PROFILE/PRIMARY_LINE/PRIMARY_LINE/CREDENTIAL/USERID", domDocument);

        orgControl.verify();
        userControl.verify();
    }

    public void testFixUserLineUrl() throws Exception {
        MockControl userControl = MockControl.createNiceControl(UserBusiness.class);
        UserBusiness user = (UserBusiness) userControl.getMock();
        UserHelper helper = new UserHelper(user);

        InputStream stream = getClass().getResourceAsStream("multiline.test.xml");
        SAXBuilder builder = new SAXBuilder();
        Document document = builder.build(stream);
        Element profile = document.getRootElement();

        Collection lines = profile.getChildren();
        for (Iterator i = lines.iterator(); i.hasNext();) {
            Element line = (Element) i.next();
            if ("51".equals(line.getAttributeValue("ref_property_id"))) {
                helper.fixUserLineUrl(line, "pgrr.somecompany.org", "pqrr.pingtel.com");
            }
        }

        DOMOutputter converter = new DOMOutputter();
        org.w3c.dom.Document domDocument = converter.output(document);

        // change the domain name that was equal to the old one
        assertXpathEvaluatesTo(
                "sip:777@pgrr.somecompany.org",
                "/PROFILE/USER_LINE[@id = '7369703a37373740646f746f2e70696e6774656c2e636f6d']/USER_LINE/URL",
                domDocument);

        // leave in place domain name that was different than the old one
        assertXpathEvaluatesTo(
                "sip:888@abc.pingtel.com",
                "/PROFILE/USER_LINE[@id = '7369703a37373740646f746f2e70696e6774656c2e636f7d']/USER_LINE/URL",
                domDocument);
    }

    public void testFixUserLineUrlString() {
        String newUrl = UserHelper.fixUserLineUrl("sipx:ddd@abc.com", "xyz.com", "abc.com");
        assertEquals("sipx:ddd@xyz.com", newUrl);
        newUrl = UserHelper.fixUserLineUrl("sipx:ddd@abc.com", "xyz.com", "abc1.com");
        assertEquals("sipx:ddd@abc.com", newUrl);
        newUrl = UserHelper.fixUserLineUrl("sipx:ddd.com@ddd.com", "xyz.com", "ddd.com");
        assertEquals("sipx:ddd.com@xyz.com", newUrl);
    }
}
