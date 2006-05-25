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
package org.sipfoundry.sipxconfig.components;

import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.ServletConfig;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.springframework.context.ApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Redirect fixed URL to configurable and complex logo url E.g. /images/logo.png to asset for logo
 * passed in SkinControl.getLogoClasspath()
 */
public class LogoAssetServlet extends HttpServlet {
    private SkinControl m_skin;

    public void init(ServletConfig config) throws ServletException {
        ApplicationContext app = WebApplicationContextUtils
                .getRequiredWebApplicationContext(config.getServletContext());
        m_skin = (SkinControl) app.getBean(SkinControl.CONTEXT_BEAN_NAME);
    }

    protected void doGet(HttpServletRequest request, HttpServletResponse response)
        throws javax.servlet.ServletException, java.io.IOException {
        InputStream in = m_skin.getLogoAsset().getResourceAsStream();
        OutputStream out = response.getOutputStream();
        IOUtils.copy(in, out);
    }

    public void destroy() {
    }
}
