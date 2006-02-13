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
package org.sipfoundry.sipxconfig.components;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.services.LinkFactory;
import org.apache.tapestry.services.ServiceConstants;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.web.WebResponse;

public class DownloadService implements IEngineService {
    public static final String SERVICE_NAME = "download";

    private static final String PARAM_PATH = "path";

    private static final String PARAM_CONTENT_TYPE = "contentType";

    private LinkFactory m_linkFactory;

    private WebResponse m_response;

    public String getName() {
        return SERVICE_NAME;
    }

    /**
     * The only parameter is the service parameters[dirName, fileName]
     */
    public void service(IRequestCycle cycle) throws IOException {
        File file = getFile(cycle);
        if (!file.canRead()) {
            return;
        }

        m_response.setHeader("Expires", "0");
        m_response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
        m_response.setHeader("Pragma", "public");
        m_response.setHeader("Content-Disposition", "attachment; filename=\"" + file.getName()
                + "\"");

        OutputStream responseOutputStream = m_response.getOutputStream(getContentType(cycle));
        InputStream stream = new FileInputStream(file);
        IOUtils.copy(stream, responseOutputStream);
    }

    /**
     * Retrieves the file object from service parameters (filename, content type)
     * 
     * @param cycle
     * @return nely created file object
     */
    private File getFile(IRequestCycle cycle) {
        String fileName = cycle.getParameter(PARAM_PATH);
        return new File(fileName);
    }

    /**
     * Retrieves the content type from service parameters
     * 
     * @param cycle
     * @return nely created file object
     */
    private ContentType getContentType(IRequestCycle cycle) {
        String contentType = cycle.getParameter(PARAM_CONTENT_TYPE);
        return new ContentType(contentType);
    }

    public ILink getLink(boolean post, Object parameter) {
        DownloadLink.Info info = (DownloadLink.Info) parameter;

        Map parameters = new HashMap();

        parameters.put(ServiceConstants.SERVICE, getName());
        parameters.put(PARAM_PATH, info.getPath());
        parameters.put(PARAM_CONTENT_TYPE, info.getContentType());

        return m_linkFactory.constructLink(this, post, parameters, false);
    }

    public void setLinkFactory(LinkFactory linkFactory) {
        m_linkFactory = linkFactory;
    }

    public void setResponse(WebResponse response) {
        m_response = response;
    }
}
