/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/util/ExTagSupport.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags.util;

import com.pingtel.pds.common.ConfigFileManager;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PDSDefinitions;

import javax.servlet.ServletConfig;
import javax.servlet.ServletContext;
import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspWriter;
import javax.servlet.jsp.PageContext;
import javax.servlet.jsp.tagext.TagSupport;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.Enumeration;

public abstract class ExTagSupport extends TagSupport {
    /**
     * Make the tag run its state cleanup method.
     */
    public int doEndTag() throws JspException {
        clearServiceState();
        return super.doEndTag();
    }

    /**
     * Make the tag run its properties cleanup method.
     *
     * In some cases will also run the state cleanup method
     */
    public void release() {
        clearServiceState();
        clearProperties();
        super.release();
    }

    /**
     */
    public void resetCustomProperties() {
        clearProperties();
    }

    /**
     * Clear the tag's properties.
     */
    protected void clearProperties() {}

    /**
     * Clear the tag's service state.
     */
    protected void clearServiceState() {}

    /**
     * 
     * @param msg
     */
    protected void log(String msg) {
        getServletContext().log(msg);
    }

    /**
     * 
     * @param msg
     * @param t
     */
    protected void log(String msg, Throwable t) {
        getServletContext().log(msg, t);
    }

    /**
     * 
     * @param name
     * 
     * @return 
     */
    protected String findInitParameter(String name) {
        String value = getInitParameter(name,
                                        PageContext.PAGE_SCOPE);
        if (null != value) {
            return value;
        }

        return getInitParameter(name, PageContext.APPLICATION_SCOPE);
    }

    /**
     * 
     * @param name
     * 
     * @return 
     */
    protected String getInitParameter(String name) {
        return getInitParameter(name,
                                PageContext.APPLICATION_SCOPE);
    }

    /**
     * 
     * @return 
     */
    protected Enumeration getInitParameterNames() {
        return getInitParameterNamesForScope(PageContext.APPLICATION_SCOPE);
    }

    /**
     * 
     * @param name
     * @param scope
     * 
     * @return 
     */
    protected String getInitParameter(String name, int scope) {
        switch (scope) {
        case PageContext.PAGE_SCOPE:
            return getServletConfig().getInitParameter(name);

        case PageContext.APPLICATION_SCOPE:
            return getServletContext().getInitParameter(name);

        default:
            throw new IllegalArgumentException("Illegal scope");
        }
    }

    /**
     * 
     * @param scope
     * 
     * @return 
     */
    protected Enumeration getInitParameterNamesForScope(int scope) {
        switch (scope) {
        case PageContext.PAGE_SCOPE:
            return getServletConfig().getInitParameterNames();

        case PageContext.APPLICATION_SCOPE:
            return getServletContext().getInitParameterNames();

        default:
            throw new IllegalArgumentException("Illegal scope");
        }
    }

    /**
     * 
     * @return 
     */
    protected ServletContext getServletContext() {
        return pageContext.getServletContext();
    }

    /**
     * 
     * @return 
     */
    protected ServletConfig getServletConfig() {
        return pageContext.getServletConfig();
    }

    /**
     * What we want to avoid is the danger that the code that we
     * will write can be interpreted as something more then plain text.
     */
    protected void writeHtml(JspWriter out, String html)
        throws IOException {
        out.print(StringUtil.escapHTML(html));
    }


    protected String getOrganizationID() {
        return (String)pageContext.getSession().getAttribute("orgid");
    }

    protected String getInstallStereoType() {
        String installStereotype = null;
        try {
            installStereotype = ConfigFileManager.getInstance().getProperty(
                        PathLocatorUtil.getInstance().getPath(
                            PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS)+
                            "pgs.props",
                        "installStereotype");
        }
        catch(FileNotFoundException e) {
            throw new RuntimeException(e);
        }

        if(installStereotype == null){
            installStereotype = PDSDefinitions.ENTERPRISE_ST;
        }

        return installStereotype;
    }
}
