/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.resource;

import java.io.InputStream;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.springframework.web.servlet.view.xslt.AbstractXsltView;
import org.w3c.dom.Document;
import org.w3c.dom.Node;

public class ResourceView extends AbstractXsltView {

    public static final String RESOURCE_STREAM = "in";

    private static final DocumentBuilderFactory FACTORY = DocumentBuilderFactory.newInstance();

    protected Node createDomNode(Map model, String root_, HttpServletRequest request_,
            HttpServletResponse response_) throws Exception {

        InputStream in = (InputStream) model.get(RESOURCE_STREAM);
        DocumentBuilder builder = FACTORY.newDocumentBuilder();
        Document doc;
        if (in != null) {
            doc = builder.parse(in);
        } else {
            doc = builder.newDocument();
        }

        return doc;
    }
}
