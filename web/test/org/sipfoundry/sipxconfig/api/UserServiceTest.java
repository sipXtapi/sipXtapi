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
package org.sipfoundry.sipxconfig.api;

import javax.xml.stream.XMLStreamException;
import javax.xml.stream.XMLStreamReader;
import javax.xml.stream.XMLStreamWriter;

import junit.framework.Test;
import junit.framework.TestCase;

import org.codehaus.xfire.attachments.Attachments;
import org.codehaus.xfire.client.ClientHandler;
import org.codehaus.xfire.client.http.SoapHttpClient;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class UserServiceTest extends TestCase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(UserServiceTest.class);
    }

    public void testEcho() throws Exception {
        ObjectHandler databinder = new ObjectHandler();
        SoapHttpClient client = new SoapHttpClient(databinder, 
                "http://localhost:9999/sipxconfig/service/UserService");
        client.setAction("echo");
        databinder.setRequest("hello");
        client.invoke();
        Object response = databinder.getResponse();
        assertNotNull(response);
    }
}

class ObjectHandler implements ClientHandler {
    private Object request;
    private Object response;

    public boolean hasRequest() {
        return true;
    }

    public void writeRequest(XMLStreamWriter writer) throws XMLStreamException {
        serialize(request, writer);
    }

    protected void serialize(Object request2, XMLStreamWriter writer) {
        try {
            writer.writeStartElement("in0");
            writer.writeCData(request2.toString());
            writer.writeEndElement();
        } catch (XMLStreamException e) {
            throw new RuntimeException(e);
        }
    }

    public void handleResponse(XMLStreamReader reader) throws XMLStreamException {
        response = deserialize(reader);
    }

    private Object deserialize(XMLStreamReader reader) {
        // TODO
        return null;
    }

    public Object getRequest() {
        return request;
    }

    public void setRequest(Object request) {
        this.request = request;
    }

    public Object getResponse() {
        return response;
    }

    public void setResponse(Object response) {
        this.response = response;
    }

    public Attachments getAttachments() {
        // TODO Auto-generated method stub
        return null;
    }
}
