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

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.engine.state.ApplicationStateManager;
import org.apache.tapestry.services.LinkFactory;
import org.apache.tapestry.services.ServiceConstants;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.site.UserSession;

public class DownloadService implements IEngineService {
    public static final String SERVICE_NAME = "download";

    private static final String PARAM_PATH = "path";

    private static final String PARAM_CONTENT_TYPE = "contentType";

    private static final String PARAM_DIGEST = "digest";

    private LinkFactory m_linkFactory;

    private FileDigestSource m_digestSource = new FileDigestSource();

    private WebResponse m_response;

    private ApplicationStateManager m_stateManager;

    public String getName() {
        return SERVICE_NAME;
    }

    public Integer getUserId() {
        if (m_stateManager.exists(UserSession.SESSION_NAME)) {
            UserSession userSession = (UserSession) m_stateManager.get(UserSession.SESSION_NAME);
            return userSession.getUserId();
        }
        return null;
    }

    /**
     * The only parameter is the service parameters[dirName, fileName]
     */
    public void service(IRequestCycle cycle) throws IOException {
        File file = getFile(cycle);
        Integer userId = getUserId();
        if (userId == null) {
            m_response.sendError(HttpServletResponse.SC_UNAUTHORIZED, file.getPath());
            return;
        }

        if (!file.canRead()) {
            m_response.sendError(HttpServletResponse.SC_NOT_FOUND, file.getPath());
            return;
        }

        String expectedMd5Digest = cycle.getParameter(PARAM_DIGEST);
        String actualMd5Digest = m_digestSource.getDigestForResource(userId, file.getPath());
        if (!actualMd5Digest.equals(expectedMd5Digest)) {
            m_response.sendError(HttpServletResponse.SC_FORBIDDEN, file.getPath());
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
        Integer userId = getUserId();
        if (userId == null) {
            throw new RuntimeException("You have to be logged in to generate download links.");
        }
        DownloadLink.Info info = (DownloadLink.Info) parameter;

        Map parameters = new HashMap();

        parameters.put(ServiceConstants.SERVICE, getName());
        parameters.put(PARAM_PATH, info.getPath());
        parameters.put(PARAM_CONTENT_TYPE, info.getContentType());
        String digest = m_digestSource.getDigestForResource(userId, info.getPath());
        parameters.put(PARAM_DIGEST, digest);

        return m_linkFactory.constructLink(this, post, parameters, false);
    }

    public void setLinkFactory(LinkFactory linkFactory) {
        m_linkFactory = linkFactory;
    }

    public void setResponse(WebResponse response) {
        m_response = response;
    }

    public void setStateManager(ApplicationStateManager stateManager) {
        m_stateManager = stateManager;
    }
}
