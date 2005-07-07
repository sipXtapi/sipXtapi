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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import javax.servlet.ServletContext;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.RequestContext;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.BackupContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BackupPage extends BasePage implements PageRenderListener {
    private static final String URL_SEPARATOR = "/";
    private static final String PATH = "admin";

    public abstract BackupContext getBackupContext();

    public abstract List getUrls();

    public abstract void setUrls(List urls);

    public void pageBeginRender(PageEvent event_) {
        List urls = getUrls();
        if (urls == null) {
            setUrls(Collections.EMPTY_LIST);
        }
    }

    public void submit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
    }

    public void backup(IRequestCycle cycle) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        ServletContext context = cycle.getRequestContext().getServlet().getServletContext();
        BackupContext backupContext = getBackupContext();
        String realPath = context.getRealPath(PATH);
        String[] backupFiles = backupContext.perform(realPath);
        if (null != backupFiles) {
            List urls = convertToUrl(backupFiles, cycle);
            setUrls(urls);
        } else {
            IValidationDelegate validator = TapestryUtils.getValidator(this);
            validator.record("Backup operation failed.", ValidationConstraint.CONSISTENCY);
        }
    }

    /**
     * Attempts to convert real path to backup files into URL from which files can be downloaded
     * 
     * A better way of handling that would be to deploy Download link service. See:
     * http://tapestry-bayeux.sourceforge.net/components/DownloadLink.html
     * 
     * @param backupFiles array of backup file names (relative to PATH)
     * @param cycle current request cycle to retriece servlet related data
     * @return List of LinkInfo object contain URL and label to create HTML link
     */
    private List convertToUrl(String[] backupFiles, IRequestCycle cycle) {
        RequestContext rc = cycle.getRequestContext();
        String contextPath = cycle.getRequestContext().getRequest().getContextPath();

        ArrayList urls = new ArrayList(backupFiles.length);
        for (int i = 0; i < backupFiles.length; i++) {
            // please note this is URL building - hence /
            String path = contextPath + URL_SEPARATOR + PATH + URL_SEPARATOR + backupFiles[i];
            String absoluteURL = rc.getAbsoluteURL(path);
            LinkInfo linkInfo = new LinkInfo(backupFiles[i], absoluteURL);
            urls.add(linkInfo);
        }
        return urls;
    }

    public static class LinkInfo {
        private String m_url;
        private String m_label;

        public LinkInfo(String label, String url) {
            m_label = label;
            m_url = url;
        }

        public String getLabel() {
            return m_label;
        }

        public String getUrl() {
            return m_url;
        }
    }
}
