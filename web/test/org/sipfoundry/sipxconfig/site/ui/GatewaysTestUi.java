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
package org.sipfoundry.sipxconfig.site.ui;

import java.io.IOException;
import java.net.MalformedURLException;

import org.xml.sax.SAXException;

import com.meterware.httpunit.GetMethodWebRequest;
import com.meterware.httpunit.SubmitButton;
import com.meterware.httpunit.WebConversation;
import com.meterware.httpunit.WebForm;
import com.meterware.httpunit.WebLink;
import com.meterware.httpunit.WebRequest;
import com.meterware.httpunit.WebResponse;
import com.meterware.httpunit.WebTable;

import junit.framework.TestCase;

/**
 * GatewaysTestUi
 */
public class GatewaysTestUi extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testAddGateways() throws Exception {
        WebResponse home = getHomePage();

        // reset page
        home = resetDialPlans(home);

        WebLink link = home.getLinkWith("List Gateways");
        WebResponse listGateways = link.click();

        WebTable gatewaysTable = listGateways.getTableWithID("list:gateway");
        int lastColumn = gatewaysTable.getColumnCount() - 1;
        assertEquals(3, lastColumn);

        WebLink addGatewayLink = listGateways.getLinkWithID("addGateway");
        WebResponse addGatewayPage = addGatewayLink.click();

        addGatewayPage = addGateway(addGatewayPage, null);
        // if validation works we are still on the same page
        assertEquals("Gateway Configuration", addGatewayPage.getTitle());

        listGateways = addGateway(addGatewayPage, "bongo");
        gatewaysTable = listGateways.getTableWithID("list:gateway");
        // we should have 2 gateway now
        assertEquals(2, gatewaysTable.getRowCount());
        assertEquals("bongoDescription", gatewaysTable.getCellAsText(1, lastColumn));

        addGatewayLink = listGateways.getLinkWithID("addGateway");
        addGatewayPage = addGatewayLink.click();

        listGateways = addGateway(addGatewayPage, "kuku");
        gatewaysTable = listGateways.getTableWithID("list:gateway");
        // we should have 2 gateway now
        assertEquals(3, gatewaysTable.getRowCount());
        assertEquals("kukuDescription", gatewaysTable.getCellAsText(2, lastColumn));
    }

    public void testDeleteGateways() throws Exception {
        WebResponse home = getHomePage();

        // reset page
        home = resetDialPlans(home);

        WebLink link = home.getLinkWith("List Gateways");
        WebResponse listGateways = link.click();

        for (int i = 0; i < 10; i++) {
            WebLink addGatewayLink = listGateways.getLinkWithID("addGateway");
            WebResponse addGatewayPage = addGatewayLink.click();

            listGateways = addGateway(addGatewayPage, "gateway" + i);
        }

        WebTable gatewaysTable = listGateways.getTableWithID("list:gateway");
        assertEquals(11, gatewaysTable.getRowCount());

        WebForm formGateway = listGateways.getFormWithID("list:gateway:form");
        SubmitButton buttonDelete = formGateway.getSubmitButtonWithID("list:gateway:delete");
        formGateway.setCheckbox("selectedRow", true);
        formGateway.setCheckbox("selectedRow$0", true);
        listGateways = formGateway.submit(buttonDelete);

        gatewaysTable = listGateways.getTableWithID("list:gateway");
        assertEquals(9, gatewaysTable.getRowCount());

        formGateway = listGateways.getFormWithID("list:gateway:form");
        buttonDelete = formGateway.getSubmitButtonWithID("list:gateway:delete");
        formGateway.setCheckbox("selectedRow", true);
        for (int i = 0; i < 7; i++) {
            formGateway.setCheckbox("selectedRow$" + i, true);
        }
        listGateways = formGateway.submit(buttonDelete);

        gatewaysTable = listGateways.getTableWithID("list:gateway");
        assertEquals(1, gatewaysTable.getRowCount());
    }

    /**
     * Fills and submits edit gateway form
     * 
     * @param page edit gateway page
     * @param name response after clicking submit button
     * @return
     */
    private WebResponse addGateway(WebResponse page, String name) throws SAXException,
            IOException {
        WebForm formGateway = page.getFormWithID("gateway");
        if (null != name) {
            formGateway.setParameter("gatewayName", name + "Name");
            formGateway.setParameter("gatewayAddress", name + "Address");
            formGateway.setParameter("gatewayDescription", name + "Description");
        }
        SubmitButton buttonSave = formGateway.getSubmitButtonWithID("gateway:save");
        return formGateway.submit(buttonSave);
    }

    /**
     * @return sipxconfig application home page
     */
    public WebResponse getHomePage() throws MalformedURLException, IOException, SAXException {
        WebConversation wc = new WebConversation();
        WebRequest req = new GetMethodWebRequest("http://localhost:8080/sipxconfig");
        WebResponse home = wc.getResponse(req);
        return home;
    }

    private WebResponse resetDialPlans(WebResponse homePage) throws IOException, SAXException {
        final WebLink resetLink = homePage.getLinkWithID("resetDialPlans");
        return resetLink.click();
    }
}
